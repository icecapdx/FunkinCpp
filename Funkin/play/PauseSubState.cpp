#include "PauseSubState.h"
#include "PlayState.h"
#include "../ui/StoryMenuState.h"
#include "../ui/NewFreeplayState.h"
#include "../game/GameConfig.h"
#include "../scripting/ScriptManager.h"
#include "input/Controls.h"
#include "song/Conductor.h"
#include "song/Song.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/math/FlxPoint.h>
#include <flixel/tweens/FlxTween.h>
#include <flixel/tweens/FlxEase.h>
#include <SDL_mixer.h>
#include <iostream>
#include <algorithm>

PauseSubState::PauseSubState()
    : bg(nullptr)
    , levelInfo(nullptr)
    , levelDifficulty(nullptr)
    , deathCounter(nullptr)
    , practiceText(nullptr)
    , pauseMusic(nullptr)
    , curSelected(0)
{
    pauseOG = {"RESUME", "RESTART SONG", "CHANGE DIFFICULTY", "TOGGLE PRACTICE MODE", "EXIT TO MENU"};
    difficultyChoices = {"EASY", "NORMAL", "HARD", "BACK"};
    menuItems = pauseOG;
}

PauseSubState::~PauseSubState() {
    destroy();
}

void PauseSubState::create() {
    ScriptManager::getInstance()->callAll(ScriptCallback::ON_PAUSE);
    
    closeCallback = []() {
        ScriptManager::getInstance()->callAll(ScriptCallback::ON_RESUME);
    };
    
    pauseMusic = new flixel::FlxSound();
    pauseMusic->loadStream(ASSETS_PATH "assets/music/breakfast.ogg", true);
    pauseMusic->setVolume(0.0f);
    pauseMusic->play();
    
    bg = new flixel::FlxSprite(0, 0);
    bg->makeGraphic(flixel::FlxG::width, flixel::FlxG::height, {0, 0, 0, 255});
    bg->alpha = 0.0f;
    bg->scrollFactor.x = 0.0f;
    bg->scrollFactor.y = 0.0f;
    bg->camera = flixel::FlxG::camera;
    
    flixel::tweens::tween(bg, {{"alpha", 0.6f}}, 0.4f, flixel::tweens::FlxEase::quadInOut);
    
    levelInfo = new flixel::FlxText(20, 15, 0, PlayState::SONG.song.c_str());
    levelInfo->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    levelInfo->setSize(32);
    levelInfo->setColor(0xFFFFFFFF);
    levelInfo->scrollFactor.x = 0.0f;
    levelInfo->scrollFactor.y = 0.0f;
    levelInfo->alpha = 0.0f;
    levelInfo->camera = flixel::FlxG::camera;
    
    const char* difficultyStrings[] = {"EASY", "NORMAL", "HARD"};
    std::string diffStr = (PlayState::storyDifficulty >= 0 && PlayState::storyDifficulty < 3) 
        ? difficultyStrings[PlayState::storyDifficulty] 
        : "NORMAL";
    
    levelDifficulty = new flixel::FlxText(20, 15 + 32, 0, diffStr.c_str());
    levelDifficulty->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    levelDifficulty->setSize(32);
    levelDifficulty->setColor(0xFFFFFFFF);
    levelDifficulty->scrollFactor.x = 0.0f;
    levelDifficulty->scrollFactor.y = 0.0f;
    levelDifficulty->alpha = 0.0f;
    levelDifficulty->camera = flixel::FlxG::camera;
    
    std::string deathText = "Blue balled: " + std::to_string(PlayState::deathCounter);
    deathCounter = new flixel::FlxText(20, 15 + 64, 0, deathText.c_str());
    deathCounter->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    deathCounter->setSize(32);
    deathCounter->setColor(0xFFFFFFFF);
    deathCounter->scrollFactor.x = 0.0f;
    deathCounter->scrollFactor.y = 0.0f;
    deathCounter->alpha = 0.0f;
    deathCounter->camera = flixel::FlxG::camera;
    
    practiceText = new flixel::FlxText(20, 15 + 64 + 32, 0, "PRACTICE MODE");
    practiceText->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    practiceText->setSize(32);
    practiceText->setColor(0xFFFFFFFF);
    practiceText->scrollFactor.x = 0.0f;
    practiceText->scrollFactor.y = 0.0f;
    practiceText->visible = PlayState::practiceMode;
    practiceText->camera = flixel::FlxG::camera;
    
    levelInfo->x = flixel::FlxG::width - (levelInfo->getWidth() + 80);
    levelDifficulty->x = flixel::FlxG::width - (levelDifficulty->getWidth() + 80);
    deathCounter->x = flixel::FlxG::width - (deathCounter->getWidth() + 80);
    practiceText->x = flixel::FlxG::width - (practiceText->getWidth() + 80);
    
    flixel::tweens::tween(levelInfo, {{"alpha", 1.0f}, {"y", 20.0f}}, 0.4f, 
        flixel::tweens::FlxEase::quadInOut, nullptr, nullptr, nullptr, 0.3f);
    
    flixel::tweens::tween(levelDifficulty, {{"alpha", 1.0f}, {"y", levelDifficulty->y + 5.0f}}, 0.4f, 
        flixel::tweens::FlxEase::quadInOut, nullptr, nullptr, nullptr, 0.5f);
    
    flixel::tweens::tween(deathCounter, {{"alpha", 1.0f}, {"y", deathCounter->y + 5.0f}}, 0.4f, 
        flixel::tweens::FlxEase::quadInOut, nullptr, nullptr, nullptr, 0.7f);
    
    regenMenu();
}

void PauseSubState::regenMenu() {
    for (auto* item : grpMenuShit) {
        if (item) delete item;
    }
    grpMenuShit.clear();
    
    for (size_t i = 0; i < menuItems.size(); i++) {
        Alphabet* songText = new Alphabet(menuItems[i], 0, (70 * i) + 30);
        songText->isMenuItem = true;
        songText->targetY = i;
        songText->camera = flixel::FlxG::camera;
        grpMenuShit.push_back(songText);
    }
    
    curSelected = 0;
    changeSelection(0);
}

void PauseSubState::update(float elapsed) {
    FlxSubState::update(elapsed);
    
    if (pauseMusic && pauseMusic->get_volume() < 0.5f) {
        pauseMusic->setVolume(pauseMusic->get_volume() + 0.01f * elapsed);
    }
    
    if (bg) bg->update(elapsed);
    if (levelInfo) levelInfo->update(elapsed);
    if (levelDifficulty) levelDifficulty->update(elapsed);
    if (deathCounter) deathCounter->update(elapsed);
    if (practiceText) practiceText->update(elapsed);
    if (pauseMusic) pauseMusic->update(elapsed);
    
    for (auto* item : grpMenuShit) {
        if (item) item->update(elapsed);
    }
    
    Controls* controls = GameConfig::getInstance()->controls;
    
    if (controls->justPressedAction(ControlAction::BACK)) {
        if (menuItems == difficultyChoices) {
            menuItems = pauseOG;
            regenMenu();
        } else {
        close();
        }
    }
    
    if (controls->justPressedAction(ControlAction::UI_UP)) {
        changeSelection(-1);
    }
    if (controls->justPressedAction(ControlAction::UI_DOWN)) {
        changeSelection(1);
    }
    
    if (controls->justPressedAction(ControlAction::ACCEPT)) {
        std::string daSelected = menuItems[curSelected];
        
        if (daSelected == "RESUME") {
            close();
        }
        else if (daSelected == "EASY" || daSelected == "NORMAL" || daSelected == "HARD") {
            int difficultyIndex;
            if (daSelected == "EASY") {
                difficultyIndex = 0;
            }
            else if (daSelected == "NORMAL") {
                difficultyIndex = 1;
            }
            else {
                difficultyIndex = 2;
            }
            
            std::string songName = PlayState::SONG.song;
            std::transform(songName.begin(), songName.end(), songName.begin(), ::tolower);
            
            std::string formattedSongName = songName;
            if (difficultyIndex == 0) {
                formattedSongName += "-easy";
            }
            else if (difficultyIndex == 2) {
                formattedSongName += "-hard";
            }
            
            PlayState::SONG = Song::loadFromJson(formattedSongName, songName);
            PlayState::storyDifficulty = difficultyIndex;            
            flixel::FlxG::game->switchState(new PlayState());
        }
        else if (daSelected == "CHANGE DIFFICULTY") {
            menuItems = difficultyChoices;
            regenMenu();
        }
        else if (daSelected == "TOGGLE PRACTICE MODE") {
            PlayState::practiceMode = !PlayState::practiceMode;
            practiceText->visible = PlayState::practiceMode;
        }
        else if (daSelected == "BACK") {
            menuItems = pauseOG;
            regenMenu();
        }
        else if (daSelected == "RESTART SONG") {
            flixel::FlxG::game->switchState(new PlayState());
        }
        else if (daSelected == "EXIT TO MENU") {
            PlayState::seenCutscene = false;
            PlayState::deathCounter = 0;
            
            if (PlayState::isStoryMode) {
                flixel::FlxG::game->switchState(new StoryMenuState());
            } else {
                flixel::FlxPoint camPos(0.0f, 0.0f);
                if (PlayState::instance) {
                    flixel::FlxObject* camFollow = PlayState::instance->getCamFollow();
                    if (camFollow) {
                        camPos.x = camFollow->x;
                        camPos.y = camFollow->y;
                    }
                }
                flixel::FlxG::game->switchState(new NewFreeplayState(true, camPos));
            }
        }
    }
}

void PauseSubState::draw() {
    if (bg) bg->draw();
    if (levelInfo) levelInfo->draw();
    if (levelDifficulty) levelDifficulty->draw();
    if (deathCounter) deathCounter->draw();
    if (practiceText && practiceText->visible) practiceText->draw();
    
    for (auto* item : grpMenuShit) {
        if (item) item->draw();
    }
    
    FlxSubState::draw();
}

void PauseSubState::changeSelection(int change) {
    if (change != 0) {
        flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
    }
    
    curSelected += change;
    
    if (curSelected < 0) {
        curSelected = static_cast<int>(menuItems.size()) - 1;
    }
    if (curSelected >= static_cast<int>(menuItems.size())) {
        curSelected = 0;
    }
    
    int bullShit = 0;
    for (auto* item : grpMenuShit) {
        if (item) {
            item->targetY = bullShit - curSelected;
            bullShit++;
            
            item->alpha = 0.6f;
            
            if (item->targetY == 0) {
                item->alpha = 1.0f;
            }
        }
    }
}

void PauseSubState::destroy() {
    if (pauseMusic) {
        pauseMusic->stop();
        delete pauseMusic;
        pauseMusic = nullptr;
    }
    
    for (auto* item : grpMenuShit) {
        if (item) delete item;
    }
    grpMenuShit.clear();
    
    if (bg) {
        delete bg;
        bg = nullptr;
    }
    if (levelInfo) {
        delete levelInfo;
        levelInfo = nullptr;
    }
    if (levelDifficulty) {
        delete levelDifficulty;
        levelDifficulty = nullptr;
    }
    if (deathCounter) {
        delete deathCounter;
        deathCounter = nullptr;
    }
    if (practiceText) {
        delete practiceText;
        practiceText = nullptr;
    }
    
    FlxSubState::destroy();
}