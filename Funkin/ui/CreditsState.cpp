#include "CreditsState.h"
#include "MainMenuState.h"
#include "../game/GameConfig.h"
#include "../play/input/Controls.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/FlxCamera.h>
#include <flixel/util/FlxColor.h>
#include <SDL_mixer.h>
#include <iostream>

CreditsState::CreditsState()
    : bg(nullptr)
    , creditsGroupY(0.0f)
    , creditsGroupHeight(0.0f)
    , scrollPaused(false)
{
    STARTING_HEIGHT = static_cast<float>(flixel::FlxG::height);
    FULL_WIDTH = static_cast<float>(flixel::FlxG::width) - (SCREEN_PAD * 2);
    
    creditsData.push_back({
        "",
        {""}
    });

    creditsData.push_back({
        "FunkinCpp",
        {"phlxsk8r"}
    });
    
    creditsData.push_back({
        "The Funkin' Crew Inc.",
        {"ninjamuffin99", "PhantomArcade", "KawaiSprite", "evilsk8r", "and everyone else who did stuff for fnf, I'm too lazy to add you all but I love you!"}
    });
    
    creditsData.push_back({
        "Special Thanks",
        {"Tom Fulp", "Newgrounds", "The Friday Night Funkin' Community"}
    });
}

CreditsState::~CreditsState() {
    destroy();
}

void CreditsState::create() {
    if (!flixel::FlxG::camera) {
        flixel::FlxG::camera = new flixel::FlxCamera(0.0f, 0.0f, 0, 0, 1.0f);
    }
    
    bg = new flixel::FlxSprite(-80, 0);
    bg->loadGraphic(ASSETS_PATH "assets/images/menuDesat.png");
    bg->scrollFactor.x = 0.0f;
    bg->scrollFactor.y = 0.0f;
    bg->setGraphicSize(static_cast<int>(bg->width * 1.18f));
    bg->updateHitbox();
    bg->screenCenter();
    bg->alpha = 0.1f;
    
    creditsGroupY = STARTING_HEIGHT;
    buildCreditsGroup();
    
    flixel::FlxG::sound.playMusic(ASSETS_PATH "assets/music/freeplayRandom.ogg", 0.0f, true);
    if (Mix_PlayingMusic()) {
        Mix_VolumeMusic(0);
        int targetVolume = static_cast<int>(0.8f * MIX_MAX_VOLUME);
        Mix_VolumeMusic(targetVolume);
    }
    
    FunkinState::create();
    
    startTransitionIn(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, -1));
}

void CreditsState::buildCreditsGroup() {
    float y = 0.0f;
    
    for (const auto& entry : creditsData) {
        if (!entry.header.empty()) {
            auto header = buildCreditsLine(entry.header, y, true);
            creditsLines.push_back(header);
            y += CREDITS_HEADER_FONT_SIZE + CREDITS_HEADER_FONT_SIZE;
        }
        
        for (const auto& line : entry.body) {
            auto textLine = buildCreditsLine(line, y, false);
            creditsLines.push_back(textLine);
            y += CREDITS_FONT_SIZE;
        }
        
        y += CREDITS_FONT_SIZE * 2.5f;
    }
    
    creditsGroupHeight = y;
}

flixel::FlxText* CreditsState::buildCreditsLine(const std::string& text, float yPos, bool header) {
    int size = header ? CREDITS_HEADER_FONT_SIZE : CREDITS_FONT_SIZE;
    
    flixel::FlxText* creditsLine = new flixel::FlxText(SCREEN_PAD, yPos, FULL_WIDTH, text.c_str());
    creditsLine->setFont(ASSETS_PATH "assets/fonts/vcr.ttf");
    creditsLine->setSize(size);
    creditsLine->setColor(0xFFFFFFFF);
    creditsLine->setBorderStyle(flixel::FlxTextBorderStyle::OUTLINE, {0, 0, 0, 255}, 2.0f);
    creditsLine->scrollFactor.x = 0.0f;
    creditsLine->scrollFactor.y = 0.0f;
    
    return creditsLine;
}

void CreditsState::update(float elapsed) {
    FunkinState::update(elapsed);
    
    Controls* controls = GameConfig::getInstance()->controls;
    
    if (!scrollPaused) {
        if (controls->pressedAction(ControlAction::ACCEPT)) {
            creditsGroupY -= CREDITS_SCROLL_FAST_SPEED * elapsed;
        } else {
            creditsGroupY -= CREDITS_SCROLL_BASE_SPEED * elapsed;
        }
        
        for (auto* line : creditsLines) {
            if (line) {}
        }
    }
    
    if (controls->justPressedAction(ControlAction::BACK) || hasEnded()) {
        exitCredits();
    }
    
    if (bg) bg->update(elapsed);
    for (auto* line : creditsLines) {
        if (line) line->update(elapsed);
    }
}

void CreditsState::draw() {
    if (bg) bg->draw();
    
    for (size_t i = 0; i < creditsLines.size(); i++) {
        auto* line = creditsLines[i];
        if (line) {
            float originalY = line->y;            
            line->y = originalY + creditsGroupY - STARTING_HEIGHT;
            
            if (line->y > -100 && line->y < flixel::FlxG::height + 100) {
                line->draw();
            }            
            line->y = originalY;
        }
    }
    
    FunkinState::draw();
}

bool CreditsState::hasEnded() {
    return creditsGroupY < -creditsGroupHeight;
}

void CreditsState::exitCredits() {
    if (scrollPaused) return;
    scrollPaused = true;
    
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
    
    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(500);
    }
    
    startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
        flixel::FlxG::game->switchState(new MainMenuState());
    });
}

void CreditsState::destroy() {
    if (bg) {
        delete bg;
        bg = nullptr;
    }
    
    for (auto* line : creditsLines) {
        if (line) delete line;
    }
    creditsLines.clear();
    
    FunkinState::destroy();
}