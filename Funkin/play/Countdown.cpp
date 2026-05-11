#include "Countdown.h"
#include "../scripting/ScriptManager.h"
#include <flixel/FlxCamera.h>
#include <iostream>

Countdown::Countdown()
    : step(0)
    , timer(0.0f)
    , interval(0.0f)
    , spriteTimer(0.0f)
    , spriteStartY(0.0f)
    , finished(false)
    , sound(nullptr)
    , sprite(nullptr)
    , soundExt(".ogg")
{
}

Countdown::~Countdown() {
    if (sound) {
        delete sound;
        sound = nullptr;
    }
    if (sprite) {
        delete sprite;
        sprite = nullptr;
    }
}

void Countdown::start(float crochet) {
    step = 0;
    timer = 0.0f;
    interval = crochet / 1000.0f;
    finished = false;
}

void Countdown::update(float elapsed) {
    if (finished) return;
    
    timer += elapsed;
    
    if (timer >= interval) {
        timer -= interval;
        nextStep();
    }
    
    if (sound) {
        sound->update(elapsed);
    }
    
    if (sprite) {
        spriteTimer += elapsed;
        float progress = spriteTimer / interval;
        if (progress > 1.0f) progress = 1.0f;
        
        sprite->alpha = 1.0f - progress;
        sprite->y = spriteStartY + (100.0f * progress);
        
        if (progress >= 1.0f) {
            delete sprite;
            sprite = nullptr;
        }
    }
}

bool Countdown::isFinished() const {
    return finished;
}

flixel::FlxSprite* Countdown::getCurrentSprite() {
    return sprite;
}

void Countdown::nextStep() {
    ScriptManager::getInstance()->callAll(ScriptCallback::ON_COUNTDOWN_TICK, {step});
    
    if (sprite) {
        delete sprite;
        sprite = nullptr;
    }
    
    switch (step) {
        case 0:
            if (sound) delete sound;
            sound = new flixel::FlxSound();
            sound->loadAsChunk(ASSETS_PATH "assets/sounds/intro3" + soundExt, false, false);
            sound->play();
            break;
            
        case 1:
            createSprite(ASSETS_PATH "assets/images/play/countdown/ready.png");
            if (sound) delete sound;
            sound = new flixel::FlxSound();
            sound->loadAsChunk(ASSETS_PATH "assets/sounds/intro2" + soundExt, false, false);
            sound->play();
            break;
            
        case 2:
            createSprite(ASSETS_PATH "assets/images/play/countdown/set.png");
            if (sound) delete sound;
            sound = new flixel::FlxSound();
            sound->loadAsChunk(ASSETS_PATH "assets/sounds/intro1" + soundExt, false, false);
            sound->play();
            break;
            
        case 3:
            createSprite(ASSETS_PATH "assets/images/play/countdown/go.png");
            if (sound) delete sound;
            sound = new flixel::FlxSound();
            sound->loadAsChunk(ASSETS_PATH "assets/sounds/introGo" + soundExt, false, false);
            sound->play();
            break;
            
        case 4:
            finished = true;
            break;
    }
    
    step++;
}

void Countdown::createSprite(const std::string& imagePath) {
    sprite = new flixel::FlxSprite();
    sprite->loadGraphic(imagePath);
    if (sprite->texture) {
        sprite->scrollFactor.x = 0.0f;
        sprite->scrollFactor.y = 0.0f;
        sprite->screenCenter();
        spriteStartY = sprite->y;
        spriteTimer = 0.0f;
    } else {
        delete sprite;
        sprite = nullptr;
    }
}