#include "StoryMenuState.h"
#include "MainMenuState.h"
#include "../play/PlayState.h"
#include "../play/song/Song.h"
#include "../play/scoring/Highscore.h"
#include "../game/GameConfig.h"
#include "../play/input/Controls.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <flixel/animation/FlxAnimationController.h>
#include <flixel/tweens/FlxTween.h>
#include <flixel/util/FlxTimer.h>
#include <flixel/util/FlxColor.h>
#include <flixel/math/FlxMath.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

static SDL_Color hexToSDLColor(uint32_t hex) {
    return SDL_Color{
        static_cast<Uint8>((hex >> 16) & 0xFF),
        static_cast<Uint8>((hex >> 8) & 0xFF),
        static_cast<Uint8>(hex & 0xFF),
        static_cast<Uint8>((hex >> 24) & 0xFF)
    };
}

StoryMenuState::StoryMenuState()
    : bg(nullptr)
    , yellowBG(nullptr)
    , blackBar(nullptr)
    , scoreText(nullptr)
    , txtWeekTitle(nullptr)
    , txtTracklist(nullptr)
    , leftArrow(nullptr)
    , sprDifficulty(nullptr)
    , rightArrow(nullptr)
    , curWeek(0)
    , curDifficulty(1)
    , lerpScore(0.0f)
    , intendedScore(0)
    , movedBack(false)
    , selectedWeek(false)
    , stopspamming(false)
{
    difficultyNames = {"EASY", "NORMAL", "HARD"};
    availableDifficulties = {1};
}

StoryMenuState::~StoryMenuState() {
    destroy();
}

void StoryMenuState::create() {
    FunkinState::create();
    
    if (!flixel::FlxG::camera) {
        flixel::FlxG::camera = new flixel::FlxCamera(0.0f, 0.0f, flixel::FlxG::width, flixel::FlxG::height, 1.0f);
    }
    
    flixel::FlxG::camera->scroll.x = 0.0f;
    flixel::FlxG::camera->scroll.y = 0.0f;
    flixel::FlxG::camera->target = nullptr;
    flixel::FlxG::camera->zoom = 1.0f;
    
    if (flixel::FlxG::sound.music && !Mix_PlayingMusic()) {
        flixel::FlxG::sound.playMusic(ASSETS_PATH "assets/music/freakyMenu.ogg");
    }
    
    loadWeeks();
    
    bg = new flixel::FlxSprite(0, 0);
    bg->makeGraphic(flixel::FlxG::width, flixel::FlxG::height, hexToSDLColor(0xFF000000));
    bg->scrollFactor.set(0, 0);
    bg->camera = flixel::FlxG::camera;
    
    yellowBG = new flixel::FlxSprite(0, 56);
    yellowBG->makeGraphic(flixel::FlxG::width, 400, hexToSDLColor(0xFFF9CF51));
    yellowBG->scrollFactor.set(0, 0);
    yellowBG->camera = flixel::FlxG::camera;
    
    blackBar = new flixel::FlxSprite(0, 0);
    blackBar->makeGraphic(flixel::FlxG::width, 56, hexToSDLColor(0xFF000000));
    blackBar->scrollFactor.set(0, 0);
    blackBar->camera = flixel::FlxG::camera;
    
    std::string xmlPath = ASSETS_PATH "assets/images/menu/story/campaign_menu_UI_assets.xml";
    std::string pngPath = ASSETS_PATH "assets/images/menu/story/campaign_menu_UI_assets.png";
    
    flixel::graphics::frames::FlxAtlasFrames* ui_tex = nullptr;
    std::ifstream xmlFile(xmlPath);
    if (xmlFile.is_open()) {
        std::stringstream buffer;
        buffer << xmlFile.rdbuf();
        std::string xmlContent = buffer.str();
        xmlFile.close();
        
        ui_tex = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(pngPath.c_str(), xmlContent);
    }
    
    for (size_t i = 0; i < weekData.size(); i++) {
        MenuItem* weekThing = new MenuItem(0, yellowBG->y + yellowBG->height + 10, weekData[i].weekFileName);
        weekThing->y += (weekThing->height + 20) * i;
        weekThing->targetY = i;
        weekThing->screenCenter(flixel::util::FlxAxes::X);
        weekThing->camera = flixel::FlxG::camera;
        grpWeekText.push_back(weekThing);
        
        if (!weekData[i].unlocked && ui_tex) {
            flixel::FlxSprite* lock = new flixel::FlxSprite(weekThing->x + weekThing->width + 10, weekThing->y);
            lock->frames = ui_tex;
            lock->texture = ui_tex->texture;
            lock->ownsTexture = false;
            
            if (!lock->animation) {
                lock->animation = new flixel::animation::FlxAnimationController();
            }
            
            auto lockFrames = ui_tex->getFramesByPrefix("lock");
            if (!lockFrames.empty()) {
                lock->animation->addByPrefix("lock", lockFrames, 24, true);
                lock->animation->play("lock");
            }
            
            lock->camera = flixel::FlxG::camera;
            grpLocks.push_back(lock);
            
            if (lock->frames && !lock->frames->frames.empty()) {
                const auto& frame = lock->frames->frames[0];
                lock->frameWidth = frame.sourceSize.w;
                lock->frameHeight = frame.sourceSize.h;
                lock->width = static_cast<float>(lock->frameWidth);
                lock->height = static_cast<float>(lock->frameHeight);
            }
        }
    }
    
    for (int char_idx = 0; char_idx < 3; char_idx++) {
        float x = (flixel::FlxG::width * 0.25f) * (1 + char_idx) - 150.0f;
        std::string charName = curWeek < weekData.size() && char_idx < static_cast<int>(weekData[curWeek].weekCharacters.size()) 
            ? weekData[curWeek].weekCharacters[char_idx] : "bf";
            
        MenuCharacter* weekCharacterThing = new MenuCharacter(x, charName);
        weekCharacterThing->y += 70;
        
        if (charName == "dad") {
            weekCharacterThing->setGraphicSize(static_cast<int>(weekCharacterThing->width * 0.5f));
            weekCharacterThing->x = -40;
            weekCharacterThing->y = -140;
        } else if (charName == "bf") {
            weekCharacterThing->setGraphicSize(static_cast<int>(weekCharacterThing->width * 0.9f));
            weekCharacterThing->x -= 80;
        } else if (charName == "gf") {
            weekCharacterThing->setGraphicSize(static_cast<int>(weekCharacterThing->width * 0.5f));
        }
        
        weekCharacterThing->camera = flixel::FlxG::camera;
        grpWeekCharacters.push_back(weekCharacterThing);
    }
    
    if (ui_tex) {
        leftArrow = new flixel::FlxSprite(0, 0);
        leftArrow->frames = ui_tex;
        leftArrow->texture = ui_tex->texture;
        leftArrow->ownsTexture = false;
        
        if (!leftArrow->animation) {
            leftArrow->animation = new flixel::animation::FlxAnimationController();
        }
        
        auto leftIdleFrames = ui_tex->getFramesByPrefix("arrow left");
        auto leftPressFrames = ui_tex->getFramesByPrefix("arrow push left");
        if (!leftIdleFrames.empty()) leftArrow->animation->addByPrefix("idle", leftIdleFrames, 24, true);
        if (!leftPressFrames.empty()) leftArrow->animation->addByPrefix("press", leftPressFrames, 24, true);
        leftArrow->animation->play("idle");
        
        if (!grpWeekText.empty()) {
            leftArrow->x = grpWeekText[0]->x + grpWeekText[0]->width + 10;
            leftArrow->y = 490.0f;
        }
        
        if (leftArrow->frames && !leftArrow->frames->frames.empty()) {
            const auto& frame = leftArrow->frames->frames[0];
            leftArrow->frameWidth = frame.sourceSize.w;
            leftArrow->frameHeight = frame.sourceSize.h;
            leftArrow->width = static_cast<float>(leftArrow->frameWidth);
            leftArrow->height = static_cast<float>(leftArrow->frameHeight);
        }
        leftArrow->scrollFactor.set(0, 0);
        leftArrow->camera = flixel::FlxG::camera;
        
        sprDifficulty = new flixel::FlxSprite(leftArrow->x + 130, leftArrow->y);
        sprDifficulty->frames = ui_tex;
        sprDifficulty->texture = ui_tex->texture;
        sprDifficulty->ownsTexture = false;
        
        if (!sprDifficulty->animation) {
            sprDifficulty->animation = new flixel::animation::FlxAnimationController();
        }
        
        auto easyFrames = ui_tex->getFramesByPrefix("EASY");
        auto normalFrames = ui_tex->getFramesByPrefix("NORMAL");
        auto hardFrames = ui_tex->getFramesByPrefix("HARD");
        if (!easyFrames.empty()) sprDifficulty->animation->addByPrefix("easy", easyFrames, 24, true);
        if (!normalFrames.empty()) sprDifficulty->animation->addByPrefix("normal", normalFrames, 24, true);
        if (!hardFrames.empty()) sprDifficulty->animation->addByPrefix("hard", hardFrames, 24, true);
        sprDifficulty->animation->play("easy");
        
        if (sprDifficulty->frames && !sprDifficulty->frames->frames.empty()) {
            const auto& frame = sprDifficulty->frames->frames[0];
            sprDifficulty->frameWidth = frame.sourceSize.w;
            sprDifficulty->frameHeight = frame.sourceSize.h;
            sprDifficulty->width = static_cast<float>(sprDifficulty->frameWidth);
            sprDifficulty->height = static_cast<float>(sprDifficulty->frameHeight);
        }
        sprDifficulty->scrollFactor.set(0, 0);
        sprDifficulty->camera = flixel::FlxG::camera;
        
        rightArrow = new flixel::FlxSprite(sprDifficulty->x + sprDifficulty->width + 50, leftArrow->y);
        rightArrow->frames = ui_tex;
        rightArrow->texture = ui_tex->texture;
        rightArrow->ownsTexture = false;
        
        if (!rightArrow->animation) {
            rightArrow->animation = new flixel::animation::FlxAnimationController();
        }
        
        auto rightIdleFrames = ui_tex->getFramesByPrefix("arrow right");
        auto rightPressFrames = ui_tex->getFramesByPrefix("arrow push right");
        if (!rightIdleFrames.empty()) rightArrow->animation->addByPrefix("idle", rightIdleFrames, 24, true);
        if (!rightPressFrames.empty()) rightArrow->animation->addByPrefix("press", rightPressFrames, 24, false);
        rightArrow->animation->play("idle");
        
        if (rightArrow->frames && !rightArrow->frames->frames.empty()) {
            const auto& frame = rightArrow->frames->frames[0];
            rightArrow->frameWidth = frame.sourceSize.w;
            rightArrow->frameHeight = frame.sourceSize.h;
            rightArrow->width = static_cast<float>(rightArrow->frameWidth);
            rightArrow->height = static_cast<float>(rightArrow->frameHeight);
        }
        rightArrow->scrollFactor.set(0, 0);
        rightArrow->camera = flixel::FlxG::camera;
    }
    
    scoreText = new flixel::FlxText(10, 10, 0, "WEEK SCORE: 0", 32);
    scoreText->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    scoreText->setSize(32);
    scoreText->setColor(0xFFFFFFFF);
    scoreText->scrollFactor.set(0, 0);
    scoreText->camera = flixel::FlxG::camera;
    
    txtWeekTitle = new flixel::FlxText(flixel::FlxG::width * 0.7f, 10, 0, "", 32);
    txtWeekTitle->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    txtWeekTitle->setSize(32);
    txtWeekTitle->setColor(0xFFFFFFFF);
    txtWeekTitle->alpha = 0.7f;
    txtWeekTitle->scrollFactor.set(0, 0);
    txtWeekTitle->camera = flixel::FlxG::camera;
    
    txtTracklist = new flixel::FlxText(flixel::FlxG::width * 0.05f, yellowBG->x + yellowBG->height + 100, 0, "Tracks", 32);
    txtTracklist->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    txtTracklist->setSize(32);
    txtTracklist->setColor(0xFFe55777);
    txtTracklist->scrollFactor.set(0, 0);
    txtTracklist->camera = flixel::FlxG::camera;
    
    changeDifficulty(0);
    updateText();
    updateAvailableDifficulties();
    
    startTransitionIn(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, -1));
}

void StoryMenuState::update(float elapsed) {
    FunkinState::update(elapsed);
    
    if (!flixel::FlxG::camera) {
        flixel::FlxG::camera = new flixel::FlxCamera(0.0f, 0.0f, flixel::FlxG::width, flixel::FlxG::height, 1.0f);
    }
    
    flixel::FlxG::camera->update(elapsed);
    
    lerpScore = flixel::math::lerp(lerpScore, static_cast<float>(intendedScore), 0.5f);
    std::string scoreStr = "WEEK SCORE: " + std::to_string(static_cast<int>(std::round(lerpScore)));
    scoreText->setText(scoreStr);
    
    if (curWeek < static_cast<int>(weekData.size())) {
        txtWeekTitle->setText(weekData[curWeek].weekName);
        txtWeekTitle->x = flixel::FlxG::width - (txtWeekTitle->getWidth() + 10);
    }
    
    if (bg) bg->update(elapsed);
    if (yellowBG) yellowBG->update(elapsed);
    if (blackBar) blackBar->update(elapsed);
    if (scoreText) scoreText->update(elapsed);
    if (txtWeekTitle) txtWeekTitle->update(elapsed);
    if (txtTracklist) txtTracklist->update(elapsed);
    
    for (auto* item : grpWeekText) {
        if (item) item->update(elapsed);
    }
    
    for (size_t i = 0; i < grpLocks.size(); i++) {
        if (grpLocks[i] && i < grpWeekText.size()) {
            grpLocks[i]->y = grpWeekText[i]->y;
            grpLocks[i]->update(elapsed);
            if (grpLocks[i]->animation) {
                grpLocks[i]->animation->update(elapsed);
            }
        }
    }
    
    for (auto* character : grpWeekCharacters) {
        if (character) character->update(elapsed);
    }
    
    
    if (leftArrow) {
        leftArrow->update(elapsed);
        if (leftArrow->animation) {
            leftArrow->animation->update(elapsed);
        }
    }
    
    if (sprDifficulty) {
        sprDifficulty->update(elapsed);
        if (sprDifficulty->animation) {
            sprDifficulty->animation->update(elapsed);
        }
    }
    
    if (rightArrow) {
        rightArrow->update(elapsed);
        if (rightArrow->animation) {
            rightArrow->animation->update(elapsed);
        }
    }
    
    if (!movedBack && !selectedWeek) {
        Controls* controls = GameConfig::getInstance()->controls;
        
        if (controls->justPressedAction(ControlAction::UI_UP)) {
            changeWeek(-1);
        }
        
        if (controls->justPressedAction(ControlAction::UI_DOWN)) {
            changeWeek(1);
        }
        
        if (controls->pressedAction(ControlAction::UI_RIGHT)) {
            if (rightArrow) rightArrow->animation->play("press");
        } else {
            if (rightArrow) rightArrow->animation->play("idle");
        }
        
        if (controls->pressedAction(ControlAction::UI_LEFT)) {
            if (leftArrow) leftArrow->animation->play("press");
        } else {
            if (leftArrow) leftArrow->animation->play("idle");
        }
        
        if (controls->justPressedAction(ControlAction::UI_RIGHT)) {
            changeDifficulty(1);
        }
        
        if (controls->justPressedAction(ControlAction::UI_LEFT)) {
            changeDifficulty(-1);
        }
        
        if (controls->justPressedAction(ControlAction::ACCEPT)) {
            selectWeek();
        }
    }
    
    if (!movedBack && !selectedWeek) {
        Controls* controls = GameConfig::getInstance()->controls;
        if (controls->justPressedAction(ControlAction::BACK)) {
            flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
            movedBack = true;
            
            startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
                flixel::FlxG::game->switchState(new MainMenuState());
            });
        }
    }
}

void StoryMenuState::draw() {
    if (bg) bg->draw();
    
    for (auto* item : grpWeekText) {
        if (item) item->draw();
    }
    
    if (blackBar) blackBar->draw();
    
    for (auto* lock : grpLocks) {
        if (lock) lock->draw();
    }
    
    if (leftArrow) leftArrow->draw();
    if (sprDifficulty) sprDifficulty->draw();
    if (rightArrow) rightArrow->draw();
    
    if (yellowBG) yellowBG->draw();
    
    for (auto* character : grpWeekCharacters) {
        if (character) character->draw();
    }
    
    if (txtTracklist) txtTracklist->draw();
    if (scoreText) scoreText->draw();
    if (txtWeekTitle) txtWeekTitle->draw();
    
    FunkinState::draw();
}

void StoryMenuState::destroy() {
    for (auto* item : grpWeekText) {
        if (item) delete item;
    }
    grpWeekText.clear();
    
    for (auto* character : grpWeekCharacters) {
        if (character) delete character;
    }
    grpWeekCharacters.clear();
    
    for (auto* lock : grpLocks) {
        if (lock) delete lock;
    }
    grpLocks.clear();
    
    if (leftArrow) {
        delete leftArrow;
        leftArrow = nullptr;
    }
    
    if (sprDifficulty) {
        delete sprDifficulty;
        sprDifficulty = nullptr;
    }
    
    if (rightArrow) {
        delete rightArrow;
        rightArrow = nullptr;
    }
    
    if (scoreText) {
        delete scoreText;
        scoreText = nullptr;
    }
    
    if (txtWeekTitle) {
        delete txtWeekTitle;
        txtWeekTitle = nullptr;
    }
    
    if (txtTracklist) {
        delete txtTracklist;
        txtTracklist = nullptr;
    }
    
    if (bg) {
        delete bg;
        bg = nullptr;
    }
    
    if (yellowBG) {
        delete yellowBG;
        yellowBG = nullptr;
    }
    
    if (blackBar) {
        delete blackBar;
        blackBar = nullptr;
    }
    
    FunkinState::destroy();
}

void StoryMenuState::loadWeeks() {
    // titsorial
    WeekData twotorial;
    twotorial.songs = { "Tutorial" };
    twotorial.weekCharacters = { "dad", "bf", "gf" };
    twotorial.weekName = "HOW TO FUNK!";
    twotorial.weekFileName = "tutorial";
    twotorial.difficulties = { 0, 1, 2 };
    twotorial.unlocked = true;

    weekData.push_back(twotorial);
    // week 1
    WeekData week1;
    week1.songs = {"Bopeebo", "Fresh", "DadBattle"};
    week1.weekCharacters = {"dad", "bf", "gf"};
    week1.weekName = "DADDY DEAREST";
    week1.weekFileName = "week1";
    week1.difficulties = {0, 1, 2};
    week1.unlocked = true;
    
    weekData.push_back(week1);
}

void StoryMenuState::changeWeek(int change) {
    curWeek += change;
    
    if (curWeek >= static_cast<int>(weekData.size())) {
        curWeek = 0;
    }
    if (curWeek < 0) {
        curWeek = weekData.size() - 1;
    }
    
    int bullShit = 0;
    for (auto* item : grpWeekText) {
        item->targetY = bullShit - curWeek;
        if (item->targetY == 0 && weekData[curWeek].unlocked) {
            item->alpha = 1.0f;
        } else {
            item->alpha = 0.6f;
        }
        bullShit++;
    }
    
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
    
    updateAvailableDifficulties();
    updateText();
}

void StoryMenuState::changeDifficulty(int change) {
    int oldDifficulty = curDifficulty;
    curDifficulty += change;
    
    if (change != 0) {
        int attempts = 0;
        while (std::find(availableDifficulties.begin(), availableDifficulties.end(), curDifficulty) == availableDifficulties.end() 
               && attempts < 3) {
            curDifficulty += change;
            if (curDifficulty < 0) curDifficulty = 2;
            if (curDifficulty > 2) curDifficulty = 0;
            attempts++;
        }
        
        if (std::find(availableDifficulties.begin(), availableDifficulties.end(), curDifficulty) == availableDifficulties.end()) {
            curDifficulty = oldDifficulty;
            return;
        }
    }
    
    if (sprDifficulty) {
        sprDifficulty->offsetX = 0;
        
        switch (curDifficulty) {
            case 0:
                sprDifficulty->animation->play("easy");
                sprDifficulty->offsetX = 20;
                break;
            case 1:
                sprDifficulty->animation->play("normal");
                sprDifficulty->offsetX = 70;
                break;
            case 2:
                sprDifficulty->animation->play("hard");
                sprDifficulty->offsetX = 20;
                break;
        }
        
        sprDifficulty->alpha = 0.0f;
        
        if (leftArrow) {
            sprDifficulty->y = leftArrow->y - 15;
            float targetY = leftArrow->y + 15;
            
            flixel::tweens::tween(sprDifficulty->getTweenTarget(), 
                {{"y", targetY}, {"alpha", 1.0f}}, 
                0.07f);
        }
    }
}

void StoryMenuState::selectWeek() {
    if (curWeek >= static_cast<int>(weekData.size())) return;
    
    if (weekData[curWeek].unlocked) {
        if (!stopspamming) {
            flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
            
            if (curWeek < static_cast<int>(grpWeekText.size())) {
                grpWeekText[curWeek]->startFlashing();
            }
            
            if (grpWeekCharacters.size() > 1 && grpWeekCharacters[1]) {
                grpWeekCharacters[1]->animation->play("bfConfirm");
            }
            
            stopspamming = true;
        }
        
        PlayState::storyPlaylist = weekData[curWeek].songs;
        PlayState::isStoryMode = true;
        selectedWeek = true;
        
        std::string diffic = "";
        switch (curDifficulty) {
            case 0:
                diffic = "-easy";
                break;
            case 2:
                diffic = "-hard";
                break;
        }
        
        PlayState::storyDifficulty = curDifficulty;
        
        std::string songName = weekData[curWeek].songs[0];
        std::transform(songName.begin(), songName.end(), songName.begin(), ::tolower);
        
        PlayState::SONG = Song::loadFromJson(songName + diffic, songName);
        PlayState::storyWeek = curWeek;
        PlayState::campaignScore = 0;
        
        flixel::util::FlxTimer* timer = new flixel::util::FlxTimer();
        timer->start(1.0f, [this](flixel::util::FlxTimer* tmr) {
            startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
                if (flixel::FlxG::sound.music) {
                    flixel::FlxG::sound.music->stop();
                }
                flixel::FlxG::game->switchState(new PlayState());
            });
            delete tmr;
        });
    }
}

void StoryMenuState::updateText() {
    if (curWeek >= static_cast<int>(weekData.size())) return;
    
    for (size_t i = 0; i < 3 && i < grpWeekCharacters.size(); i++) {
        if (grpWeekCharacters[i] && i < weekData[curWeek].weekCharacters.size()) {
            grpWeekCharacters[i]->animation->play(weekData[curWeek].weekCharacters[i]);
        }
    }
    
    if (grpWeekCharacters.size() > 0 && grpWeekCharacters[0]) {
        std::string animName = grpWeekCharacters[0]->animation->current;
        
        if (animName == "parents-christmas") {
            grpWeekCharacters[0]->offsetX = -200;
            grpWeekCharacters[0]->offsetY = -200;
            grpWeekCharacters[0]->setGraphicSize(static_cast<int>(grpWeekCharacters[0]->width * 0.99f));
        } else if (animName == "senpai") {
            grpWeekCharacters[0]->offsetX = -130;
            grpWeekCharacters[0]->offsetY = 0;
            grpWeekCharacters[0]->setGraphicSize(static_cast<int>(grpWeekCharacters[0]->width * 1.4f));
        } else if (animName == "mom") {
            grpWeekCharacters[0]->offsetX = -100;
            grpWeekCharacters[0]->offsetY = -200;
            grpWeekCharacters[0]->setGraphicSize(static_cast<int>(grpWeekCharacters[0]->width * 1.0f));
        } else if (animName == "dad") {
            grpWeekCharacters[0]->offsetX = -120;
            grpWeekCharacters[0]->offsetY = -200;
        } else if (animName == "tankman") {
            grpWeekCharacters[0]->offsetX = -60;
            grpWeekCharacters[0]->offsetY = 20;
            grpWeekCharacters[0]->setGraphicSize(static_cast<int>(grpWeekCharacters[0]->width * 1.0f));
        } else {
            grpWeekCharacters[0]->offsetX = -100;
            grpWeekCharacters[0]->offsetY = -100;
            grpWeekCharacters[0]->setGraphicSize(static_cast<int>(grpWeekCharacters[0]->width * 1.0f));
        }
    }
    
    std::string tracklistText = "Tracks\n";
    for (const auto& song : weekData[curWeek].songs) {
        tracklistText += "\n" + song;
    }
    
    std::transform(tracklistText.begin(), tracklistText.end(), tracklistText.begin(), ::toupper);
    
    if (txtTracklist) {
        txtTracklist->setText(tracklistText);
        txtTracklist->x = (flixel::FlxG::width - txtTracklist->getWidth()) / 2.0f;
        txtTracklist->x -= flixel::FlxG::width * 0.35f;
    }
    
    intendedScore = Highscore::getWeekScore(curWeek, curDifficulty);
}

void StoryMenuState::updateAvailableDifficulties() {
    if (curWeek < static_cast<int>(weekData.size())) {
        availableDifficulties = weekData[curWeek].difficulties;
        if (availableDifficulties.empty()) {
            availableDifficulties = {1};
        }
    } else {
        availableDifficulties = {1};
    }
    
    std::sort(availableDifficulties.begin(), availableDifficulties.end());
    
    if (std::find(availableDifficulties.begin(), availableDifficulties.end(), curDifficulty) == availableDifficulties.end()) {
        curDifficulty = availableDifficulties[0];
        changeDifficulty(0);
    }
}