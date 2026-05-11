#include "NewFreeplayState.h"
#include "MainMenuState.h"
#include "../play/song/Conductor.h"
#include "../play/song/SongLoader.h"
#include "../play/PlayState.h"
#include "../play/scoring/Highscore.h"
#include "../game/GameConfig.h"
#include "../play/input/Controls.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <flixel/animation/FlxAnimationController.h>
#include <flixel/math/FlxMath.h>
#include <flixel/sound/FlxSound.h>
#include <flixel/text/FlxText.h>
#include <flixel/tweens/FlxTweenUtil.h>
#include <flixel/util/FlxColor.h>
#include <SDL.h>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <fstream>

NewFreeplayState::NewFreeplayState(bool _transitionFromMenu, flixel::FlxPoint camFollowPos)
    : transitionFromMenu(_transitionFromMenu), curSelected(0), curDifficulty(1), curCategory(0),
      transitionOver(false), waitForFirstUpdateToStart(true), pendingSongStart(false), songStartTimer(0.0f),
      exitingToMenu(false), exitTimer(0.0f),
      albumTime(0.0f), curAlbum("vol1"),
      lerpScore(0), intendedScore(0), prevScore(0), prevAccuracy(0), musicStartTicks(0)
{
    camFollow = new flixel::FlxObject(camFollowPos.x, camFollowPos.y, 1, 1);
    camTarget = flixel::FlxPoint(0.0f, 0.0f);
    
    bg = nullptr;
    flash = nullptr;
    fakeMenuBg = nullptr;
    cover = nullptr;
    topBar = nullptr;
    freeplayText = nullptr;
    highscoreSprite = nullptr;
    clearPercentSprite = nullptr;
    scoreDisplay = nullptr;
    percentDisplay = nullptr;
    albumTitle = nullptr;
    arrowLeft = nullptr;
    arrowRight = nullptr;
    difficulty = nullptr;
    categoryTitle = nullptr;
    miniArrowLeft = nullptr;
    miniArrowRight = nullptr;
    album = nullptr;
    albumDummy = nullptr;
    dj = nullptr;
    camMenu = nullptr;
    camFreeplay = nullptr;
}

NewFreeplayState::~NewFreeplayState() {
    destroy();
}

void NewFreeplayState::create() {
    if (transitionFromMenu) {
        flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
        
        if (flixel::FlxG::sound.music && Mix_PlayingMusic()) {
            Mix_VolumeMusic(0);
        }
    }
    
    camMenu = new flixel::FlxCamera(0.0f, 0.0f, flixel::FlxG::width, flixel::FlxG::height, 1.0f);
    flixel::FlxG::camera = camMenu;
    
    camFreeplay = new flixel::FlxCamera(0, 0, flixel::FlxG::width, flixel::FlxG::height, 1.0f);
    camFreeplay->bgColor.setAlpha(0);
    
    fakeMainMenuSetup();
    
    addScrollingText("HOT BLOODED IN MORE WAYS THAN ONE ", 168.0f, 6.8f, 43, 0xFFFFF383);
    addScrollingText("BOYFRIEND ", 220.0f, -3.8f, 60, 0xFFFF9963);
    addScrollingText("PROTECT YO NUTS ", 285.0f, 3.5f, 43, 0xFFFFFFFF);
    addScrollingText("BOYFRIEND ", 335.0f, -3.8f, 60, 0xFFFF9963);
    addScrollingText("HOT BLOODED IN MORE WAYS THAN ONE ", 397.0f, 6.8f, 43, 0xFFFFF383);
    addScrollingText("BOYFRIEND ", 455.0f, -3.8f, 60, 0xFFFEA400);
    
    addSong("tutorial", "Tutorial", "gf", 0, "vol1", { "ALL", "Tutorial" });
    addSong("bopeebo", "Bopeebo", "dad", 1, "vol1", {"ALL", "Week 1"});
    addSong("fresh", "Fresh", "dad", 1, "vol1", {"ALL", "Week 1"});
    addSong("dadbattle", "Dad Battle", "dad", 1, "vol1", {"ALL", "Week 1"});
    addSong("fnf2", "FNF2", "bf", 2, "vol1", {"ALL", "Extras"});
    addSong("shitton-of-notes", "A shitton of notes", "bf", 2, "vol1", {"ALL", "Extras"});
    
    FunkinState::create();
}

void NewFreeplayState::update(float elapsed) {
    if (waitForFirstUpdateToStart) {
        createFreeplayStuff();
        waitForFirstUpdateToStart = false;
    }
    
    if (Mix_PlayingMusic() && musicStartTicks > 0) {
        Conductor::songPosition = static_cast<float>(SDL_GetTicks() - musicStartTicks);
    }
    
    albumTime += elapsed;
    if (albumTime >= albumPeriod) {
        albumTime = 0.0f;
        if (album && albumDummy) {
            album->x = albumDummy->x;
            album->y = albumDummy->y;
            album->angle = albumDummy->angle;
        }
    }
    
    lerpScore = static_cast<int>(flixel::math::lerp(static_cast<float>(lerpScore), static_cast<float>(intendedScore), 0.4f));
    if (std::abs(lerpScore - intendedScore) <= 10) {
        lerpScore = intendedScore;
    }
    
    if (scoreDisplay && !scoreDisplay->isTweening) {
       scoreDisplay->setNumber(lerpScore);
    }
    
    Controls* controls = GameConfig::getInstance()->controls;
    
    bool upP = controls->justPressedAction(ControlAction::UI_UP);
    bool downP = controls->justPressedAction(ControlAction::UI_DOWN);
    bool leftP = controls->justPressedAction(ControlAction::UI_LEFT);
    bool rightP = controls->justPressedAction(ControlAction::UI_RIGHT);
    bool backP = controls->justPressedAction(ControlAction::BACK);
    bool accepted = controls->justPressedAction(ControlAction::ACCEPT);
    
    bool qP = flixel::FlxG::keys.keys[SDL_SCANCODE_Q].justPressed() || 
              flixel::FlxG::gamepads.justPressed(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    bool eP = flixel::FlxG::keys.keys[SDL_SCANCODE_E].justPressed() || 
              flixel::FlxG::gamepads.justPressed(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    bool oneP = flixel::FlxG::keys.keys[SDL_SCANCODE_1].justPressed();
    
    if (oneP && scoreDisplay) {
        scoreDisplay->tweenNumber(3194585, 0.8f);
    }
    
    if (transitionOver) {
        if (upP) {
            changeSelected(-1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        } else if (downP) {
            changeSelected(1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        }
        
        if (leftP) {
            changeDifficulty(-1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        } else if (rightP) {
            changeDifficulty(1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        }
        
        if (qP) {
            changeCategory(-1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        } else if (eP) {
            changeCategory(1);
            flixel::FlxSound* scrollSound = new flixel::FlxSound();
            scrollSound->loadAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            scrollSound->play();
            delete scrollSound;
        }
        
        if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
            for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
                updateCapsulePosition(static_cast<int>(i));
            }
        }
        
        if (leftP && arrowLeft) {
            arrowLeft->setScale(0.8f, 0.8f);
        } else if (arrowLeft) {
            arrowLeft->setScale(1.0f, 1.0f);
        }
        
        if (rightP && arrowRight) {
            arrowRight->setScale(0.8f, 0.8f);
        } else if (arrowRight) {
            arrowRight->setScale(1.0f, 1.0f);
        }
        
        if (qP && miniArrowLeft) {
            miniArrowLeft->setScale(0.6f, 0.6f);
        } else if (miniArrowLeft) {
            miniArrowLeft->setScale(1.0f, 1.0f);
        }
        
        if (eP && miniArrowRight) {
            miniArrowRight->setScale(0.6f, 0.6f);
        } else if (miniArrowRight) {
            miniArrowRight->setScale(1.0f, 1.0f);
        }
        
        if (backP) {
            transitionOver = false;
            exitingToMenu = true;
            exitTimer = 0.0f;
            
            flixel::FlxSound* cancelSound = new flixel::FlxSound();
            cancelSound->loadAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
            cancelSound->play();
            delete cancelSound;
            
            if (Mix_PlayingMusic()) {
                Mix_FadeOutMusic(500);
            }
            
            exitAnimation();
        }
        
        if (accepted) {
            transitionOver = false;
            
            setUpScrollingTextAccept();
            refreshScrollingText();
            
            if (flash) {
                if (flixel::tweens::globalManager) {
                    flixel::tweens::globalManager->completeTweensOf(flash);
                }
                flash->alpha = 1.0f;
                flash->visible = true;
                flixel::tweens::tweenAlpha(flash, 0.0f, 1.0f, flixel::tweens::FlxEase::linear, nullptr, 0.1f);
            }
            
            flixel::FlxSound* confirmSound = new flixel::FlxSound();
            confirmSound->loadAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
            confirmSound->play();
            delete confirmSound;
            
            if (dj && dj->animation) {
                dj->animation->play("confirm", true);
            }
            
            if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
                if (curSelected >= 0 && curSelected < static_cast<int>(categoryMap[categoryNames[curCategory]].size())) {
                    categoryMap[categoryNames[curCategory]][curSelected]->confirm();
                    
                    pendingSongStart = true;
                    songStartTimer = 0.0f;
                }
            }
        }
    }
    
    if (pendingSongStart) {
        songStartTimer += elapsed;
        if (songStartTimer >= songStartDelay) {
            pendingSongStart = false;
            
            if (Mix_PlayingMusic()) {
                Mix_FadeOutMusic(500);
            }
            
            startSong();
        }
    }
    
    if (exitingToMenu) {
        exitTimer += elapsed;
        float transitionEndTime = transitionTimeExit + (staggerTimeExit * 4.0f);
        
        if (exitTimer >= transitionEndTime - 0.4f && exitTimer < transitionEndTime) {
            static bool fadeStarted = false;
            if (!fadeStarted) {
                fadeStarted = true;
                startTransitionOut(0.4f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), nullptr);
            }
        }
        
        if (exitTimer >= transitionEndTime) {
            exitingToMenu = false;
            flixel::FlxG::game->switchState(new MainMenuState());
        }
    }
    
    if (camFollow) {
        camFollow->x = camTarget.x;
        camFollow->y = camTarget.y;
    }
    
    if (camMenu) {
        camMenu->update(elapsed);
    }
    if (camFreeplay) {
        camFreeplay->update(elapsed);
    }
    
    if (fakeMenuBg) fakeMenuBg->update(elapsed);
    for (auto item : fakeMenuItems) {
        if (item) item->update(elapsed);
    }
    
    if (scoreDisplay) scoreDisplay->update(elapsed);
    if (percentDisplay) percentDisplay->update(elapsed);
    if (bg) bg->update(elapsed);
    if (flash) flash->update(elapsed);
    if (cover) cover->update(elapsed);
    if (topBar) topBar->update(elapsed);
    if (freeplayText) freeplayText->update(elapsed);
    if (highscoreSprite) {
        highscoreSprite->update(elapsed);
        if (highscoreSprite->animation) highscoreSprite->animation->update(elapsed);
    }
    if (clearPercentSprite) clearPercentSprite->update(elapsed);
    if (albumTitle) albumTitle->update(elapsed);
    if (arrowLeft) {
        arrowLeft->update(elapsed);
        if (arrowLeft->animation) arrowLeft->animation->update(elapsed);
    }
    if (arrowRight) {
        arrowRight->update(elapsed);
        if (arrowRight->animation) arrowRight->animation->update(elapsed);
    }
    if (difficulty) difficulty->update(elapsed);
    if (categoryTitle) categoryTitle->update(elapsed);
    if (miniArrowLeft) miniArrowLeft->update(elapsed);
    if (miniArrowRight) miniArrowRight->update(elapsed);
    if (album) album->update(elapsed);
    if (dj) {
        dj->update(elapsed);
        if (dj->animation) dj->animation->update(elapsed);
        
        if (dj->animation && dj->animation->current == "idle") {
            dj->offsetX = 0.0f;
            dj->offsetY = 0.0f;
        } else if (dj->animation && dj->animation->current == "intro") {
            dj->offsetX = 5.0f;
            dj->offsetY = 427.0f;
        } else if (dj->animation && dj->animation->current == "confirm") {
            dj->offsetX = 43.0f;
            dj->offsetY = -24.0f;
        }
    }
    
    if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        for (auto capsule : categoryMap[categoryNames[curCategory]]) {
            capsule->update(elapsed);
        }
    }
    
    for (auto scrollText : scrollingTexts) {
        if (scrollText) scrollText->update(elapsed);
    }
    
    FunkinState::update(elapsed);

    if (flixel::FlxG::camera) {
        flixel::FlxG::camera->update(elapsed);
    }
}

void NewFreeplayState::draw() {
    if (fakeMenuBg) fakeMenuBg->draw();
    for (auto item : fakeMenuItems) {
        if (item) item->draw();
    }
    
    if (bg) bg->draw();
    
    for (auto scrollText : scrollingTexts) {
        if (scrollText && scrollText->visible) {
            scrollText->draw();
        }
    }
    
    if (flash) flash->draw();
    if (cover) cover->draw();
    
    if (dj) dj->draw();
    
    if (highscoreSprite) highscoreSprite->draw();
    if (clearPercentSprite) clearPercentSprite->draw();
    if (scoreDisplay) scoreDisplay->draw();
    if (percentDisplay) percentDisplay->draw();
    if (album) album->draw();
    if (albumTitle) albumTitle->draw();
    
    if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        for (auto capsule : categoryMap[categoryNames[curCategory]]) {
            if (capsule->capsuleSprite) capsule->capsuleSprite->draw();
            if (capsule->text) capsule->text->draw();
            if (capsule->icon) capsule->icon->draw();
        }
    }
    
    if (arrowLeft) arrowLeft->draw();
    if (arrowRight) arrowRight->draw();
    if (difficulty) difficulty->draw();
    if (categoryTitle) categoryTitle->draw();
    if (miniArrowLeft) miniArrowLeft->draw();
    if (miniArrowRight) miniArrowRight->draw();
    
    if (topBar) topBar->draw();
    if (freeplayText) freeplayText->draw();
    
    FunkinState::draw();
}

void NewFreeplayState::addScrollingText(const std::string& text, float yPos, float scrollSpeed, int size, Uint32 color) {
    flixel::FlxText* tempText = new flixel::FlxText(0, 0, 0, text.c_str());
    tempText->setFont(ASSETS_PATH "assets/fonts/5by7.ttf");
    tempText->setSize(size);
    tempText->setColor(color);
    
    SDL_Texture* textTexture = tempText->getTexture();
    
    flixel::addons::FlxBackdrop* backdrop = new flixel::addons::FlxBackdrop(textTexture, 1.0f, 0.0f, true, false);
    backdrop->y = yPos;
    backdrop->velocity.x = scrollSpeed * 30.0f;
    backdrop->scrollFactor.x = 0.0f;
    backdrop->scrollFactor.y = 0.0f;
    backdrop->camera = nullptr;
    backdrop->visible = false;
    scrollingTexts.push_back(backdrop);    
    scrollingTextObjects.push_back(tempText);
}

void NewFreeplayState::setUpScrollingTextAccept() {
    for (auto scrollText : scrollingTexts) {
        if (scrollText) { delete scrollText; }
    }
    scrollingTexts.clear();
    
    for (auto textObj : scrollingTextObjects) {
        if (textObj) { delete textObj; }
    }
    scrollingTextObjects.clear();
}

void NewFreeplayState::refreshScrollingText() {
    addScrollingText("DON'T FUCK THIS ONE UP ", 168.0f, 6.8f, 43, 0xFFFFF383);
    addScrollingText("LET'S GO ", 220.0f, -3.8f, 60, 0xFFFF9963);
    addScrollingText("YOU GOT THIS ", 285.0f, 3.5f, 43, 0xFFFFFFFF);
    addScrollingText("LET'S GO ", 335.0f, -3.8f, 60, 0xFFFF9963);
    addScrollingText("DON'T FUCK THIS ONE UP ", 397.0f, 6.8f, 43, 0xFFFFF383);
    addScrollingText("LET'S GO ", 455.0f, -3.8f, 60, 0xFFFEA400);
    
    for (auto scrollText : scrollingTexts) {
        if (scrollText) scrollText->visible = true;
    }
}

void NewFreeplayState::destroy() {
    for (auto scrollText : scrollingTexts) {
        if (scrollText) { delete scrollText; }
    }
    scrollingTexts.clear();
    
    for (auto textObj : scrollingTextObjects) {
        if (textObj) { delete textObj; }
    }
    scrollingTextObjects.clear();
    
    if (bg) { delete bg; bg = nullptr; }
    if (flash) { delete flash; flash = nullptr; }
    if (fakeMenuBg) { delete fakeMenuBg; fakeMenuBg = nullptr; }
    for (auto item : fakeMenuItems) {
        if (item) { delete item; }
    }
    fakeMenuItems.clear();
    if (cover) { delete cover; cover = nullptr; }
    if (topBar) { delete topBar; topBar = nullptr; }
    if (freeplayText) { delete freeplayText; freeplayText = nullptr; }
    if (highscoreSprite) { delete highscoreSprite; highscoreSprite = nullptr; }
    if (clearPercentSprite) { delete clearPercentSprite; clearPercentSprite = nullptr; }
    if (scoreDisplay) { delete scoreDisplay; scoreDisplay = nullptr; }
    if (percentDisplay) { delete percentDisplay; percentDisplay = nullptr; }
    if (albumTitle) { delete albumTitle; albumTitle = nullptr; }
    if (arrowLeft) { delete arrowLeft; arrowLeft = nullptr; }
    if (arrowRight) { delete arrowRight; arrowRight = nullptr; }
    if (difficulty) { delete difficulty; difficulty = nullptr; }
    if (categoryTitle) { delete categoryTitle; categoryTitle = nullptr; }
    if (categoryFont) { delete categoryFont; categoryFont = nullptr; }
    if (miniArrowLeft) { delete miniArrowLeft; miniArrowLeft = nullptr; }
    if (miniArrowRight) { delete miniArrowRight; miniArrowRight = nullptr; }
    if (album) { delete album; album = nullptr; }
    if (albumDummy) { delete albumDummy; albumDummy = nullptr; }
    if (dj) { delete dj; dj = nullptr; }
    if (camFollow) { delete camFollow; camFollow = nullptr; }    
    if (camFreeplay) { delete camFreeplay; camFreeplay = nullptr; }
    
    for (auto capsule : capsules) {
        delete capsule;
    }
    capsules.clear();
    categoryMap.clear();
    categoryNames.clear();
}

void NewFreeplayState::beatHit() {
    if (Mix_PlayingMusic() && curBeat % 2 == 0 && dj && dj->animation && dj->animation->current == "idle") {
        dj->animation->play("idle", true);
    }
    
    FunkinState::beatHit();
}

void NewFreeplayState::createFreeplayStuff() {
    bg = new flixel::FlxSprite(0.0f, 0.0f);
    bg->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/bgs/yellow.png");
    bg->scrollFactor.x = 0.0f;
    bg->scrollFactor.y = 0.0f;
    bg->camera = camMenu;
    
    flash = new flixel::FlxSprite(0.0f, 0.0f);
    flash->makeGraphic(1280, 720, SDL_Color{255, 255, 255, 255});
    flash->alpha = 0.0f;
    flash->visible = false;
    flash->scrollFactor.x = 0.0f;
    flash->scrollFactor.y = 0.0f;
    flash->camera = camMenu;
    
    cover = new flixel::FlxSprite(1280.0f, 0.0f);
    cover->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/covers/dad.png");
    cover->x -= cover->width;
    cover->scrollFactor.x = 0.0f;
    cover->scrollFactor.y = 0.0f;
    cover->camera = camMenu;
    
    topBar = new flixel::FlxSprite(0.0f, 0.0f);
    topBar->makeGraphic(1280, 64, SDL_Color{0, 0, 0, 255});
    topBar->scrollFactor.x = 0.0f;
    topBar->scrollFactor.y = 0.0f;
    topBar->camera = camMenu;
    
    freeplayText = new flixel::FlxText(16.0f, 16.0f, 0, "FREEPLAY", 32);
    freeplayText->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    freeplayText->setColor(0xFFFFFFFF);
    freeplayText->scrollFactor.x = 0.0f;
    freeplayText->scrollFactor.y = 0.0f;
    freeplayText->camera = camMenu;
    
    highscoreSprite = new flixel::FlxSprite(860.0f, 70.0f);
    std::string hsXml = ASSETS_PATH "assets/images/menu/freeplay/highscore.xml";
    std::string hsPng = ASSETS_PATH "assets/images/menu/freeplay/highscore.png";
    std::ifstream hsXmlFile(hsXml);
    std::stringstream hsBuffer;
    hsBuffer << hsXmlFile.rdbuf();
    auto hsFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(hsPng.c_str(), hsBuffer.str());
    if (hsFrames) {
        highscoreSprite->frames = hsFrames;
        highscoreSprite->texture = hsFrames->texture;
        highscoreSprite->ownsTexture = false;
        highscoreSprite->animation = new flixel::animation::FlxAnimationController();
        auto hsIndices = hsFrames->getFramesByPrefix("");
        highscoreSprite->animation->addByPrefix("loop", hsIndices, 24, true);
        highscoreSprite->animation->play("loop");
    }
    highscoreSprite->scrollFactor.x = 0.0f;
    highscoreSprite->scrollFactor.y = 0.0f;
    highscoreSprite->camera = camMenu;
    
    clearPercentSprite = new flixel::FlxSprite(1165.0f, 65.0f);
    clearPercentSprite->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/clearBox.png");
    clearPercentSprite->scrollFactor.x = 0.0f;
    clearPercentSprite->scrollFactor.y = 0.0f;
    clearPercentSprite->camera = camMenu;
    
    scoreDisplay = new DigitDisplay(956.0f, 178.0f, "menu/freeplay/digital_numbers", 7, 0.4f, -25.0f);
    scoreDisplay->setDigitOffset(1, 45.0f);
    scoreDisplay->scrollFactor.x = 0.0f;
    scoreDisplay->scrollFactor.y = 0.0f;
    scoreDisplay->camera = camMenu;
    
    percentDisplay = new DigitDisplay(1150.0f, 87.0f, "menu/freeplay/clearText", 3, 1.0f, 3.0f, 0, true);
    percentDisplay->setDigitOffset(1, -16.0f);
    percentDisplay->scrollFactor.x = 0.0f;
    percentDisplay->scrollFactor.y = 0.0f;
    percentDisplay->camera = camMenu;
    
    albumDummy = new flixel::FlxObject(950.0f, 285.0f, 1, 1);
    albumDummy->angle = 10.0f;
    album = new flixel::FlxSprite(albumDummy->x, albumDummy->y);
    album->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/album/vol1/album.png");
    album->angle = albumDummy->angle;
    album->scrollFactor.x = 0.0f;
    album->scrollFactor.y = 0.0f;
    album->camera = camMenu;
    
    albumTitle = new flixel::FlxSprite(album->x - 5.0f, album->y + 205.0f);
    albumTitle->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/album/vol1/title.png");
    albumTitle->scrollFactor.x = 0.0f;
    albumTitle->scrollFactor.y = 0.0f;
    albumTitle->camera = camMenu;
    
    arrowLeft = new flixel::FlxSprite(20.0f, 70.0f);
    std::string arrowXml = ASSETS_PATH "assets/images/menu/freeplay/freeplaySelector.xml";
    std::string arrowPng = ASSETS_PATH "assets/images/menu/freeplay/freeplaySelector.png";
    std::ifstream arrowXmlFile(arrowXml);
    std::stringstream arrowBuffer;
    arrowBuffer << arrowXmlFile.rdbuf();
    auto arrowFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(arrowPng.c_str(), arrowBuffer.str());
    if (arrowFrames) {
        arrowLeft->frames = arrowFrames;
        arrowLeft->texture = arrowFrames->texture;
        arrowLeft->ownsTexture = false;
        arrowLeft->animation = new flixel::animation::FlxAnimationController();
        auto arrowIndices = arrowFrames->getFramesByPrefix("arrow pointer loop");
        arrowLeft->animation->addByPrefix("loop", arrowIndices, 24, true);
        arrowLeft->animation->play("loop");
    }
    arrowLeft->scrollFactor.x = 0.0f;
    arrowLeft->scrollFactor.y = 0.0f;
    arrowLeft->camera = camMenu;
    
    arrowRight = new flixel::FlxSprite(325.0f, 70.0f);
    if (arrowFrames) {
        arrowRight->frames = arrowFrames;
        arrowRight->texture = arrowFrames->texture;
        arrowRight->ownsTexture = false;
        arrowRight->animation = new flixel::animation::FlxAnimationController();
        auto arrowIndices = arrowFrames->getFramesByPrefix("arrow pointer loop");
        arrowRight->animation->addByPrefix("loop", arrowIndices, 24, true);
        arrowRight->animation->play("loop");
        arrowRight->flipX = true;
    }
    arrowRight->scrollFactor.x = 0.0f;
    arrowRight->scrollFactor.y = 0.0f;
    arrowRight->camera = camMenu;
    
    difficulty = new flixel::FlxSprite(197.0f, 115.0f);
    difficulty->loadGraphic((ASSETS_PATH "assets/images/menu/freeplay/diff/" + diffNumberToDiffName(curDifficulty) + ".png").c_str());
    if (difficulty->width > 0 && difficulty->height > 0) {
        difficulty->offsetX = difficulty->width / 2.0f;
        difficulty->offsetY = difficulty->height / 2.0f;
    }
    difficulty->scrollFactor.x = 0.0f;
    difficulty->scrollFactor.y = 0.0f;
    difficulty->camera = camMenu;
    
    std::string fontLetters = "AaBbCcDdEeFfGgHhiIJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz:1234567890";
    categoryFont = flixel::FlxBitmapFont::fromMonospace(ASSETS_PATH "assets/images/menu/freeplay/resultFont.png", fontLetters, 49, 62);
    
    categoryTitle = new flixel::FlxBitmapText(categoryFont);
    categoryTitle->letterSpacing = -15;
    categoryTitle->setText(categoryNames.size() > 0 ? categoryNames[curCategory] : "ALL");
    categoryTitle->screenCenter();
    categoryTitle->y = 85.0f;
    categoryTitle->scrollFactor.x = 0.0f;
    categoryTitle->scrollFactor.y = 0.0f;
    categoryTitle->camera = camMenu;
    
    float titleCenterY = categoryTitle->y + (categoryTitle->height / 2.0f);
    
    miniArrowLeft = new flixel::FlxSprite(categoryTitle->x, titleCenterY);
    miniArrowLeft->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/miniArrow.png");
    miniArrowLeft->x -= miniArrowLeft->width;
    miniArrowLeft->y -= miniArrowLeft->height / 2.0f;
    miniArrowLeft->y -= 7.0f;
    miniArrowLeft->x -= 20.0f;
    miniArrowLeft->flipX = true;
    miniArrowLeft->scrollFactor.x = 0.0f;
    miniArrowLeft->scrollFactor.y = 0.0f;
    miniArrowLeft->camera = camMenu;
    
    miniArrowRight = new flixel::FlxSprite(categoryTitle->x + categoryTitle->width, titleCenterY);
    miniArrowRight->loadGraphic(ASSETS_PATH "assets/images/menu/freeplay/miniArrow.png");
    miniArrowRight->y -= miniArrowRight->height / 2.0f;
    miniArrowRight->x += 20.0f;
    miniArrowRight->y -= 7.0f;
    miniArrowRight->scrollFactor.x = 0.0f;
    miniArrowRight->scrollFactor.y = 0.0f;
    miniArrowRight->camera = camMenu;
    
    dj = new flixel::FlxSprite(-9.0f, 290.0f);
    std::string djXml = ASSETS_PATH "assets/images/menu/freeplay/dj/bf.xml";
    std::string djPng = ASSETS_PATH "assets/images/menu/freeplay/dj/bf.png";
    std::ifstream djXmlFile(djXml);
    std::stringstream djBuffer;
    djBuffer << djXmlFile.rdbuf();
    auto djFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(djPng.c_str(), djBuffer.str());
    if (djFrames) {
        dj->frames = djFrames;
        dj->texture = djFrames->texture;
        dj->ownsTexture = false;
        dj->animation = new flixel::animation::FlxAnimationController();
        
        auto idleIndices = djFrames->getFramesByPrefix("Boyfriend DJ");
        auto introIndices = djFrames->getFramesByPrefix("boyfriend dj intro");
        auto confirmIndices = djFrames->getFramesByPrefix("Boyfriend DJ confirm");
        
        dj->animation->addByPrefix("idle", idleIndices, 24, false);
        dj->animation->addByPrefix("intro", introIndices, 24, false);
        dj->animation->addByPrefix("confirm", confirmIndices, 24, false);
        
        auto djPtr = dj;
        auto thisPtr = this;
        dj->animation->finishCallback = [djPtr, thisPtr](const std::string& name) {
            if (name == "idle") {
                auto it = djPtr->animation->animations.find("idle");
                if (it != djPtr->animation->animations.end() && it->second.frames.size() >= 4) {
                    int startFrame = static_cast<int>(it->second.frames.size()) - 4;
                    djPtr->animation->play("idle", true, startFrame);
                }
            } else if (name == "intro") {
                if (thisPtr->transitionFromMenu && !thisPtr->transitionOver) {
                    djPtr->animation->play("idle", true);
                    thisPtr->djIntroFinish();
                }
            }
        };
        
        if (transitionFromMenu) {
            dj->animation->play("intro", true);
        } else {
            dj->animation->play("idle", true);
        }
    }
    dj->camera = camFreeplay;
    
    addCapsules();
    calcAvailableDifficulties();
    updateScore();
    updateAccuracy();
    updateAlbum(false);
    
    if (camMenu && camFollow) {
        camMenu->target = camFollow;
        camMenu->followLerp = 0.04f;
        camMenu->deadzone = nullptr;
        
        if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
            if (curSelected >= 0 && curSelected < static_cast<int>(categoryMap[categoryNames[curCategory]].size())) {
                auto capsule = categoryMap[categoryNames[curCategory]][curSelected];
                camTarget.x = capsule->x + capsule->targetPos.x;
                camTarget.y = capsule->y + capsule->targetPos.y;
            }
        }
        
        camFollow->x = camTarget.x;
        camFollow->y = camTarget.y;
        camMenu->focusOn(flixel::FlxPoint(camFollow->x, camFollow->y));
    }
    
    if (transitionFromMenu) {
        if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
            for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
                updateCapsulePosition(static_cast<int>(i));
            }
        }
        
        if (bg) bg->x -= 1280.0f;
        if (flash) flash->visible = true;
        if (cover) cover->x += 1280.0f;
        if (topBar) topBar->y -= 720.0f;
        if (freeplayText) freeplayText->y -= 720.0f;
        if (highscoreSprite) highscoreSprite->x += 1280.0f;
        if (clearPercentSprite) clearPercentSprite->x += 1280.0f;
        if (scoreDisplay) scoreDisplay->x += 1280.0f;
        if (percentDisplay) percentDisplay->x += 1280.0f;
        if (albumTitle) albumTitle->x += 1280.0f;
        if (arrowLeft) arrowLeft->y -= 720.0f;
        if (arrowRight) arrowRight->y -= 720.0f;
        if (difficulty) difficulty->y -= 720.0f;
        if (categoryTitle) categoryTitle->y -= 720.0f;
        if (miniArrowLeft) miniArrowLeft->y -= 720.0f;
        if (miniArrowRight) miniArrowRight->y -= 720.0f;
        
        float albumPosX = albumDummy->x;
        if (albumDummy) {
            albumDummy->x = 1280.0f;
            albumDummy->angle = 70.0f;
        }
        if (album) {
            album->x = albumDummy->x;
            album->angle = albumDummy->angle;
        }
        
        float transitionTime = 1.0f;
        float staggerTime = 0.1f;
        
        if (bg) flixel::tweens::tweenX(bg, 0.0f, transitionTime, flixel::tweens::FlxEase::quintOut);
        if (cover) {
            float coverTargetX = cover->x - 1280.0f;
            flixel::tweens::tweenX(cover, coverTargetX, transitionTime, flixel::tweens::FlxEase::quintOut);
        }
        if (topBar) flixel::tweens::tweenY(topBar, 0.0f, transitionTime, flixel::tweens::FlxEase::quintOut);
        if (freeplayText) flixel::tweens::tweenY(freeplayText, 16.0f, transitionTime, flixel::tweens::FlxEase::quintOut);
        
        if (highscoreSprite) {
            float hsTargetX = highscoreSprite->x - 1280.0f;
            auto tween = flixel::tweens::tweenX(highscoreSprite, hsTargetX, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        if (clearPercentSprite) {
            float cpTargetX = clearPercentSprite->x - 1280.0f;
            auto tween = flixel::tweens::tweenX(clearPercentSprite, cpTargetX, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime * 2.0f;
        }
        if (scoreDisplay) {
            float sdTargetX = scoreDisplay->x - 1280.0f;
            auto tween = new flixel::tweens::VarTween(transitionTime);
            tween->object = scoreDisplay;
            tween->ease = flixel::tweens::FlxEase::quintOut;
            tween->type = flixel::tweens::FlxTweenType::ONESHOT;
            tween->startDelay = staggerTime * 3.0f;
            tween->addProperty("x", &scoreDisplay->x, sdTargetX);
            tween->start();
            if (flixel::tweens::globalManager) {
                flixel::tweens::globalManager->tweens.push_back(tween);
            }
        }
        if (percentDisplay) {
            float pdTargetX = percentDisplay->x - 1280.0f;
            auto tween = new flixel::tweens::VarTween(transitionTime);
            tween->object = percentDisplay;
            tween->ease = flixel::tweens::FlxEase::quintOut;
            tween->type = flixel::tweens::FlxTweenType::ONESHOT;
            tween->startDelay = staggerTime * 2.0f;
            tween->addProperty("x", &percentDisplay->x, pdTargetX);
            tween->start();
            if (flixel::tweens::globalManager) {
                flixel::tweens::globalManager->tweens.push_back(tween);
            }
        }
        if (albumTitle) {
            float atTargetX = albumTitle->x - 1280.0f;
            auto tween = flixel::tweens::tweenX(albumTitle, atTargetX, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        
        if (arrowLeft) {
            float alTargetY = arrowLeft->y + 720.0f;
            auto tween = flixel::tweens::tweenY(arrowLeft, alTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        if (arrowRight) {
            float arTargetY = arrowRight->y + 720.0f;
            auto tween = flixel::tweens::tweenY(arrowRight, arTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        if (difficulty) {
            float dTargetY = difficulty->y + 720.0f;
            auto tween = flixel::tweens::tweenY(difficulty, dTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime * 2.0f;
        }
        if (categoryTitle) {
            float ctTargetY = categoryTitle->y + 720.0f;
            auto tween = flixel::tweens::tweenY(categoryTitle, ctTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime * 2.0f;
        }
        if (miniArrowLeft) {
            float malTargetY = miniArrowLeft->y + 720.0f;
            auto tween = flixel::tweens::tweenY(miniArrowLeft, malTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        if (miniArrowRight) {
            float marTargetY = miniArrowRight->y + 720.0f;
            auto tween = flixel::tweens::tweenY(miniArrowRight, marTargetY, transitionTime, flixel::tweens::FlxEase::quintOut);
            if (tween) tween->startDelay = staggerTime;
        }
        
        if (albumDummy) {
            auto tweenX = new flixel::tweens::VarTween(transitionTime / 1.1f);
            tweenX->object = albumDummy;
            tweenX->ease = flixel::tweens::FlxEase::elasticOut;
            tweenX->type = flixel::tweens::FlxTweenType::ONESHOT;
            tweenX->addProperty("x", &albumDummy->x, albumPosX);
            tweenX->start();
            if (flixel::tweens::globalManager) {
                flixel::tweens::globalManager->tweens.push_back(tweenX);
            }
            
            auto tweenAngle = new flixel::tweens::VarTween(transitionTime / 1.1f);
            tweenAngle->object = albumDummy;
            tweenAngle->ease = flixel::tweens::FlxEase::elasticOut;
            tweenAngle->type = flixel::tweens::FlxTweenType::ONESHOT;
            tweenAngle->addProperty("angle", &albumDummy->angle, 10.0f);
            tweenAngle->start();
            if (flixel::tweens::globalManager) {
                flixel::tweens::globalManager->tweens.push_back(tweenAngle);
            }
        }
        
        if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
            for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
                auto capsule = categoryMap[categoryNames[curCategory]][i];
                if (capsule) {
                    capsule->xPositionOffset = 1000.0f;
                    capsule->snapToTargetPos();
                    
                    float delay = std::max(0.0f, (staggerTime / 4.0f) * (static_cast<int>(i) + 1 - curSelected));
                    auto tween = new flixel::tweens::VarTween(transitionTime);
                    tween->object = capsule;
                    tween->ease = flixel::tweens::FlxEase::quintOut;
                    tween->type = flixel::tweens::FlxTweenType::ONESHOT;
                    tween->startDelay = delay;
                    tween->addProperty("xPositionOffset", &capsule->xPositionOffset, 0.0f);
                    tween->start();
                    
                    if (flixel::tweens::globalManager) {
                        flixel::tweens::globalManager->tweens.push_back(tween);
                    }
                }
            }
        }
    } else {
        djIntroFinish();
    }
}

void NewFreeplayState::fakeMainMenuSetup() {
    fakeMenuBg = new flixel::FlxSprite(-80, 0);
    fakeMenuBg->loadGraphic(ASSETS_PATH "assets/images/menuBG.png");
    fakeMenuBg->scrollFactor.x = 0.0f;
    fakeMenuBg->scrollFactor.y = 0.18f;
    fakeMenuBg->setGraphicSize(static_cast<int>(fakeMenuBg->width * 1.18f));
    fakeMenuBg->updateHitbox();
    fakeMenuBg->screenCenter();
    fakeMenuBg->camera = camMenu;
    
    std::vector<std::string> menuItemNames = {"storymode", "freeplay", "options", "credits"};
    
    for (size_t i = 0; i < menuItemNames.size(); i++) {
        AtlasMenuItem* item = new AtlasMenuItem(menuItemNames[i], ASSETS_PATH "assets/images/menu/mainmenu/" + menuItemNames[i], nullptr);
        item->ID = static_cast<int>(i);
        item->centered = true;
        item->changeAnim("idle");
        item->scrollFactor.x = 0.0f;
        item->scrollFactor.y = 0.4f;
        item->camera = camMenu;
        fakeMenuItems.push_back(item);
    }
    
    int spacing = 160;
    int top = (flixel::FlxG::height - (spacing * (static_cast<int>(fakeMenuItems.size()) - 1))) / 2;
    for (size_t i = 0; i < fakeMenuItems.size(); i++) {
        fakeMenuItems[i]->x = flixel::FlxG::width / 2.0f;
        fakeMenuItems[i]->y = static_cast<float>(top + spacing * i);
        
        if (fakeMenuItems[i]->centered && fakeMenuItems[i]->frames) {
            fakeMenuItems[i]->x -= fakeMenuItems[i]->width / 2.0f;
        }
        
        if (i == 1) {
            fakeMenuItems[i]->changeAnim("selected");
        }
    }
}

void NewFreeplayState::djIntroFinish() {
    transitionOver = true;
    startFreeplaySong();
    
    if (flash) {
        flash->alpha = 1.0f;
        flash->visible = true;
        flixel::tweens::tweenAlpha(flash, 0.0f, 1.0f, flixel::tweens::FlxEase::linear, nullptr, 0.1f);
    }
    
    for (auto scrollText : scrollingTexts) {
        if (scrollText) scrollText->visible = true;
    }
    
    if (camFollow) {
        camFollow->x = camTarget.x;
        camFollow->y = camTarget.y;
    }
}

void NewFreeplayState::startFreeplaySong() {
    Conductor::songPosition = 0.0f;
    Conductor::bpmChangeMap.clear();
    Conductor::changeBPM(145.0f);
    flixel::FlxG::sound.playMusic(ASSETS_PATH "assets/music/freeplayRandom.ogg", 0.9f, true);
    musicStartTicks = SDL_GetTicks();
}

void NewFreeplayState::addSong(const std::string& _song, const std::string& _displayName, const std::string& _icon, int _week, const std::string& _album, const std::vector<std::string>& categories) {
    Capsule* capsule = new Capsule(_song, _displayName, _icon, _week, _album);
    capsules.push_back(capsule);
    
    for (const auto& cat : categories) {
        if (categoryMap.find(cat) == categoryMap.end()) {
            categoryNames.push_back(cat);
            categoryMap[cat] = std::vector<Capsule*>();
        }
        categoryMap[cat].push_back(capsule);
    }
}

void NewFreeplayState::addCapsules() {
    for (auto capsule : capsules) {
        if (capsule->capsuleSprite) capsule->capsuleSprite->camera = camFreeplay;
        if (capsule->icon) capsule->icon->camera = camFreeplay;
        if (capsule->text) capsule->text->camera = camFreeplay;
    }
    
    if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
            auto capsule = categoryMap[categoryNames[curCategory]][i];
            updateCapsulePosition(static_cast<int>(i));
            capsule->snapToTargetPos();
        }
    }
}

void NewFreeplayState::updateCapsulePosition(int index) {
    if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        categoryMap[categoryNames[curCategory]][index]->targetPos.x = categoryMap[categoryNames[curCategory]][index]->intendedX(index - curSelected);
        categoryMap[categoryNames[curCategory]][index]->targetPos.y = categoryMap[categoryNames[curCategory]][index]->intendedY(index - curSelected);
    }
}

void NewFreeplayState::changeSelected(int change) {
    if (categoryNames.size() == 0 || curCategory >= static_cast<int>(categoryNames.size())) return;
    
    curSelected += change;
    if (curSelected < 0) {
        curSelected = static_cast<int>(categoryMap[categoryNames[curCategory]].size()) - 1;
    } else if (curSelected >= static_cast<int>(categoryMap[categoryNames[curCategory]].size())) {
        curSelected = 0;
    }
    
    calcAvailableDifficulties();
    updateScore();
    updateAccuracy();
    updateAlbum();
}

void NewFreeplayState::changeDifficulty(int change) {
    curDifficulty += change;
    if (curDifficulty < 0) curDifficulty = 2;
    if (curDifficulty > 2) curDifficulty = 0;
    
    if (std::find(allowedDifficulties.begin(), allowedDifficulties.end(), curDifficulty) == allowedDifficulties.end()) {
        if (allowedDifficulties.size() > 0) {
            curDifficulty = allowedDifficulties[0];
        }
    }
    
    if (difficulty) {
        float oldY = difficulty->y;
        difficulty->y = oldY - 15.0f;
        difficulty->alpha = 0.5f;
        difficulty->loadGraphic((ASSETS_PATH "assets/images/menu/freeplay/diff/" + diffNumberToDiffName(curDifficulty) + ".png").c_str());
        
        flixel::tweens::tweenY(difficulty, oldY, 0.2f, flixel::tweens::FlxEase::backOut);
        flixel::tweens::tweenAlpha(difficulty, 1.0f, 0.2f, flixel::tweens::FlxEase::sineOut);
    }
    
    updateScore();
    updateAccuracy();
}

void NewFreeplayState::changeCategory(int change) {
    curCategory += change;
    if (curCategory < 0) {
        curCategory = static_cast<int>(categoryNames.size()) - 1;
    } else if (curCategory >= static_cast<int>(categoryNames.size())) {
        curCategory = 0;
    }
    
    if (categoryTitle && categoryNames.size() > 0) {
        categoryTitle->letterSpacing = -15;
        categoryTitle->setText(categoryNames[curCategory]);
        categoryTitle->screenCenter();
        
        float titleCenterY = categoryTitle->y + (categoryTitle->height / 2.0f);
        
        if (miniArrowLeft) {
            miniArrowLeft->x = categoryTitle->x;
            miniArrowLeft->x -= miniArrowLeft->width;
            miniArrowLeft->x -= 20.0f;
            miniArrowLeft->y = titleCenterY - (miniArrowLeft->height / 2.0f) - 7.0f;
        }
        
        if (miniArrowRight) {
            miniArrowRight->x = categoryTitle->x + categoryTitle->width;
            miniArrowRight->x += 20.0f;
            miniArrowRight->y = titleCenterY - (miniArrowRight->height / 2.0f) - 7.0f;
        }
        
        float oldY = categoryTitle->y;
        categoryTitle->y = oldY - 15.0f;
        flixel::tweens::tweenY(categoryTitle, oldY, 0.1f, flixel::tweens::FlxEase::cubeOut);
    }
    
    curSelected = 0;
    calcAvailableDifficulties();
    updateScore();
    updateAccuracy();
    updateAlbum();
    addCapsules();
    tweenCapsulesOnScreen(0.5f, 0.02f, 0.1f, 400.0f);
}

void NewFreeplayState::updateScore() {
    if (categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
            if (static_cast<int>(i) == curSelected) {
                categoryMap[categoryNames[curCategory]][i]->select();
                
                auto capsule = categoryMap[categoryNames[curCategory]][i];               
                std::string songName = capsule->song;
                std::transform(songName.begin(), songName.end(), songName.begin(), ::tolower);
                intendedScore = Highscore::getScore(songName, curDifficulty);
                camTarget.x = capsule->x + 400.0f;
                camTarget.y = capsule->y + 200.0f;
            } else {
                categoryMap[categoryNames[curCategory]][i]->deselect();
            }
        }
    }
}

void NewFreeplayState::updateAccuracy() {
    if (percentDisplay && categoryNames.size() > 0 && curCategory < static_cast<int>(categoryNames.size())) {
        if (curSelected >= 0 && curSelected < static_cast<int>(categoryMap[categoryNames[curCategory]].size())) {
            auto capsule = categoryMap[categoryNames[curCategory]][curSelected];            
            std::string songName = capsule->song;
            std::transform(songName.begin(), songName.end(), songName.begin(), ::tolower);
            float accuracy = Highscore::getAccuracy(songName, curDifficulty);            
            int accuracyPercent = static_cast<int>(accuracy * 100.0f);
            percentDisplay->tweenNumber(accuracyPercent, 0.8f);
        }
    }
}

void NewFreeplayState::updateAlbum(bool doTween) {
    if (categoryNames.size() == 0 || curCategory >= static_cast<int>(categoryNames.size())) return;
    if (curSelected < 0 || curSelected >= static_cast<int>(categoryMap[categoryNames[curCategory]].size())) return;
    
    std::string newAlbum = categoryMap[categoryNames[curCategory]][curSelected]->album;
    if (newAlbum != curAlbum) {
        curAlbum = newAlbum;
        if (album) {
            float startScale = 0.8f;
            album->scale.x = startScale;
            album->scale.y = startScale;
            album->alpha = 0.0f;
            album->loadGraphic((ASSETS_PATH "assets/images/menu/freeplay/album/" + curAlbum + "/album.png").c_str());
            
            if (doTween) {
                flixel::tweens::tweenScaleX(album, 1.0f, 0.4f, flixel::tweens::FlxEase::elasticOut);
                flixel::tweens::tweenScaleY(album, 1.0f, 0.4f, flixel::tweens::FlxEase::elasticOut);
                flixel::tweens::tweenAlpha(album, 1.0f, 0.3f, flixel::tweens::FlxEase::sineOut);
            } else {
                album->scale.x = 1.0f;
                album->scale.y = 1.0f;
                album->alpha = 1.0f;
            }
        }
        if (albumTitle) {
            float titleY = albumTitle->y;
            albumTitle->y = titleY + 20.0f;
            albumTitle->alpha = 0.0f;
            albumTitle->loadGraphic((ASSETS_PATH "assets/images/menu/freeplay/album/" + curAlbum + "/title.png").c_str());
            
            if (doTween) {
                flixel::tweens::tweenY(albumTitle, titleY, 0.3f, flixel::tweens::FlxEase::backOut);
                flixel::tweens::tweenAlpha(albumTitle, 1.0f, 0.3f, flixel::tweens::FlxEase::sineOut);
            } else {
                albumTitle->y = titleY;
                albumTitle->alpha = 1.0f;
            }
        }
    }
}

void NewFreeplayState::calcAvailableDifficulties() {
    allowedDifficulties.clear();
    allowedDifficulties.push_back(0);
    allowedDifficulties.push_back(1);
    allowedDifficulties.push_back(2);
}

std::string NewFreeplayState::diffNumberToDiffName(int diff) {
    switch (diff) {
        case 0: return "easy";
        case 1: return "normal";
        case 2: return "hard";
        default: return "normal";
    }
}

std::string NewFreeplayState::capitalizeFirstLetter(const std::string& str) {
    if (str.empty()) return str;
    std::string result = str;
    result[0] = std::toupper(result[0]);
    return result;
}

void NewFreeplayState::tweenCapsulesOnScreen(float _transitionTime, float _randomVariation, float _staggerTime, float _distance) {
    if (categoryNames.size() == 0 || curCategory >= static_cast<int>(categoryNames.size())) return;
    
    for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
        auto capsule = categoryMap[categoryNames[curCategory]][i];
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->completeTweensOf(capsule);
        }
        capsule->xPositionOffset = _distance;
        capsule->snapToTargetPos();
        
        float delay = std::max(0.0f, (_staggerTime / 4.0f) * (static_cast<int>(i) + 1 - curSelected));
        auto tween = new flixel::tweens::VarTween(_transitionTime);
        tween->object = capsule;
        tween->ease = flixel::tweens::FlxEase::quintOut;
        tween->type = flixel::tweens::FlxTweenType::ONESHOT;
        tween->startDelay = delay;
        tween->addProperty("xPositionOffset", &capsule->xPositionOffset, 0.0f);
        tween->start();
        
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tween);
        }
    }
}

void NewFreeplayState::tweenCapsulesOffScreen(float _transitionTime, float _randomVariation, float _staggerTime, float _distance) {
    if (categoryNames.size() == 0 || curCategory >= static_cast<int>(categoryNames.size())) return;
    
    for (size_t i = 0; i < categoryMap[categoryNames[curCategory]].size(); i++) {
        auto capsule = categoryMap[categoryNames[curCategory]][i];
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->completeTweensOf(capsule);
        }
        
        float delay = std::max(0.0f, (_staggerTime / 4.0f) * (static_cast<int>(i) + 1 - curSelected));
        auto tween = new flixel::tweens::VarTween(_transitionTime);
        tween->object = capsule;
        tween->ease = flixel::tweens::FlxEase::cubeIn;
        tween->type = flixel::tweens::FlxTweenType::ONESHOT;
        tween->startDelay = delay;
        tween->addProperty("xPositionOffset", &capsule->xPositionOffset, _distance);
        tween->start();
        
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tween);
        }
    }
}

void NewFreeplayState::exitAnimation() {
    if (bg) flixel::tweens::tweenX(bg, bg->x - 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 3.0f);
    if (cover) flixel::tweens::tweenX(cover, cover->x + 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 3.0f);
    if (dj) flixel::tweens::tweenX(dj, dj->x - 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    if (topBar) flixel::tweens::tweenY(topBar, -720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 3.0f);
    if (freeplayText) flixel::tweens::tweenY(freeplayText, 16.0f - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 3.0f);
    if (highscoreSprite) flixel::tweens::tweenX(highscoreSprite, highscoreSprite->x + 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 2.0f);
    if (clearPercentSprite) flixel::tweens::tweenX(clearPercentSprite, clearPercentSprite->x + 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    
    if (scoreDisplay) {
        auto tween = new flixel::tweens::VarTween(transitionTimeExit);
        tween->object = scoreDisplay;
        tween->ease = flixel::tweens::FlxEase::cubeIn;
        tween->type = flixel::tweens::FlxTweenType::ONESHOT;
        tween->startDelay = staggerTimeExit * 0.0f;
        tween->addProperty("x", &scoreDisplay->x, scoreDisplay->x + 1280.0f);
        tween->start();
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tween);
        }
    }
    
    if (percentDisplay) {
        auto tween = new flixel::tweens::VarTween(transitionTimeExit);
        tween->object = percentDisplay;
        tween->ease = flixel::tweens::FlxEase::cubeIn;
        tween->type = flixel::tweens::FlxTweenType::ONESHOT;
        tween->startDelay = staggerTimeExit * 1.0f;
        tween->addProperty("x", &percentDisplay->x, percentDisplay->x + 1280.0f);
        tween->start();
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tween);
        }
    }
    
    if (albumTitle) flixel::tweens::tweenX(albumTitle, albumTitle->x + 1280.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 2.0f);
    if (arrowLeft) flixel::tweens::tweenY(arrowLeft, arrowLeft->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    if (arrowRight) flixel::tweens::tweenY(arrowRight, arrowRight->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    if (difficulty) flixel::tweens::tweenY(difficulty, difficulty->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 2.0f);
    if (categoryTitle) flixel::tweens::tweenY(categoryTitle, categoryTitle->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 2.0f);
    if (miniArrowLeft) flixel::tweens::tweenY(miniArrowLeft, miniArrowLeft->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    if (miniArrowRight) flixel::tweens::tweenY(miniArrowRight, miniArrowRight->y - 720.0f, transitionTimeExit, flixel::tweens::FlxEase::cubeIn, nullptr, staggerTimeExit * 1.0f);
    
    if (albumDummy) {
        auto tweenX = new flixel::tweens::VarTween(transitionTimeExit / 1.1f);
        tweenX->object = albumDummy;
        tweenX->ease = flixel::tweens::FlxEase::elasticOut;
        tweenX->type = flixel::tweens::FlxTweenType::ONESHOT;
        tweenX->startDelay = staggerTimeExit * 3.0f;
        tweenX->addProperty("x", &albumDummy->x, 1380.0f);
        tweenX->start();
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tweenX);
        }
        
        auto tweenAngle = new flixel::tweens::VarTween(transitionTimeExit / 1.1f);
        tweenAngle->object = albumDummy;
        tweenAngle->ease = flixel::tweens::FlxEase::elasticOut;
        tweenAngle->type = flixel::tweens::FlxTweenType::ONESHOT;
        tweenAngle->startDelay = staggerTimeExit * 3.0f;
        tweenAngle->addProperty("angle", &albumDummy->angle, 70.0f);
        tweenAngle->start();
        if (flixel::tweens::globalManager) {
            flixel::tweens::globalManager->tweens.push_back(tweenAngle);
        }
    }
    
    tweenCapsulesOffScreen(transitionTimeExit, 0.03f, staggerTimeExit);
    
    for (auto scrollText : scrollingTexts) {
        if (scrollText) { delete scrollText; }
    }
    scrollingTexts.clear();
    
    for (auto textObj : scrollingTextObjects) {
        if (textObj) { delete textObj; }
    }
    scrollingTextObjects.clear();
    
    if (flash && flixel::tweens::globalManager) {
        flixel::tweens::globalManager->completeTweensOf(flash);
        flash->alpha = 1.0f;
        flash->visible = true;
        flixel::tweens::tweenAlpha(flash, 0.0f, 0.5f, flixel::tweens::FlxEase::linear, nullptr, 0.1f);
    }
}

void NewFreeplayState::startSong() {
    if (categoryNames.size() == 0 || curCategory >= static_cast<int>(categoryNames.size())) return;
    if (curSelected < 0 || curSelected >= static_cast<int>(categoryMap[categoryNames[curCategory]].size())) return;
    
    std::string songName = categoryMap[categoryNames[curCategory]][curSelected]->song;
    
    std::string formattedSongName = songName;
    if (curDifficulty == 0) {
        formattedSongName += "-easy";
    } else if (curDifficulty == 2) {
        formattedSongName += "-hard";
    }
    
    PlayState::SONG = Song::loadFromJson(formattedSongName, songName);
    PlayState::storyDifficulty = curDifficulty;
    PlayState::isStoryMode = false;
    
    Conductor::songPosition = 0.0f;
    Conductor::changeBPM(100.0f);
    
    startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
        PlayState* playState = new PlayState();
        flixel::FlxG::game->switchState(playState);
    });
}