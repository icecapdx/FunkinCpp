#include "PopUpStuff.h"
#include <flixel/FlxG.h>
#include <flixel/util/FlxAxes.h>
#include <cstdlib>
#include <iostream>

PopUpStuff::PopUpStuff() {
}

PopUpStuff::~PopUpStuff() {
    clear();
}

void PopUpStuff::popUpScore(const std::string& rating, int combo, float crochet, flixel::FlxCamera* camera) {
    int windowWidth = flixel::FlxG::width;
    int windowHeight = flixel::FlxG::height;
    
    flixel::FlxSprite* ratingSprite = new flixel::FlxSprite();
    ratingSprite->loadGraphic(ASSETS_PATH "assets/images/play/ratings/" + rating + ".png");
    ratingSprite->screenCenter(flixel::util::FlxAxes::XY);
    ratingSprite->x = windowWidth * 0.55f - 40;
    ratingSprite->y -= 60;
    ratingSprite->acceleration.y = 550;
    ratingSprite->velocity.y = -(std::rand() % 36 + 140);
    ratingSprite->velocity.x = -(std::rand() % 11);
    ratingSprite->setScale(0.7f, 0.7f);
    ratingSprite->scrollFactor.x = 0.0f;
    ratingSprite->scrollFactor.y = 0.0f;
    ratingSprite->alpha = 1.0f;
    if (camera) {
        ratingSprite->camera = camera;
    }
    sprites.push_back(ratingSprite);
    timers.push_back(0.0f);
    
    if (combo >= 10 || combo == 0) {
        flixel::FlxSprite* comboSprite = new flixel::FlxSprite();
        comboSprite->loadGraphic(ASSETS_PATH "assets/images/play/numbers/combo.png");
        comboSprite->screenCenter(flixel::util::FlxAxes::XY);
        comboSprite->x = windowWidth * 0.55f;
        comboSprite->acceleration.y = 600;
        comboSprite->velocity.y = -150;
        comboSprite->velocity.x = std::rand() % 10 + 1;
        comboSprite->setScale(0.7f, 0.7f);
        comboSprite->scrollFactor.x = 0.0f;
        comboSprite->scrollFactor.y = 0.0f;
        comboSprite->alpha = 1.0f;
        if (camera) {
            comboSprite->camera = camera;
        }
        sprites.push_back(comboSprite);
        timers.push_back(0.0f);
        
        int hundreds = combo / 100;
        int tens = (combo % 100) / 10;
        int ones = combo % 10;
        int comboDigits[3] = {hundreds, tens, ones};
        
        for (int i = 0; i < 3; i++) {
            if (combo >= 10 || combo == 0) {
                flixel::FlxSprite* numSprite = new flixel::FlxSprite();
                numSprite->loadGraphic(ASSETS_PATH "assets/images/play/numbers/num" + std::to_string(comboDigits[i]) + ".png");
                numSprite->screenCenter(flixel::util::FlxAxes::XY);
                numSprite->x = windowWidth * 0.55f + (43 * i) - 90;
                numSprite->y += 80;
                numSprite->acceleration.y = std::rand() % 101 + 200;
                numSprite->velocity.y = -(std::rand() % 21 + 140);
                numSprite->velocity.x = ((std::rand() % 11) - 5.0f);
                numSprite->setScale(0.5f, 0.5f);
                numSprite->scrollFactor.x = 0.0f;
                numSprite->scrollFactor.y = 0.0f;
                numSprite->alpha = 1.0f;
                if (camera) {
                    numSprite->camera = camera;
                }
                sprites.push_back(numSprite);
                timers.push_back(0.0f);
            }
        }
    }
}

void PopUpStuff::update(float elapsed, float crochet) {
    for (size_t i = 0; i < sprites.size(); ) {
        timers[i] += elapsed;
        
        float startDelay;
        if (i == 0) {
            startDelay = crochet * 0.001f;
        } else if (i == 1) {
            startDelay = crochet * 0.001f;
        } else {
            startDelay = crochet * 0.002f;
        }
        
        float fadeTime = 0.2f;
        float totalTime = startDelay + fadeTime;
        
        if (timers[i] >= totalTime) {
            delete sprites[i];
            sprites.erase(sprites.begin() + i);
            timers.erase(timers.begin() + i);
        } else {
            sprites[i]->update(elapsed);
            
            if (timers[i] < startDelay) {
                sprites[i]->alpha = 1.0f;
            } else {
                float fadeProgress = (timers[i] - startDelay) / fadeTime;
                sprites[i]->alpha = 1.0f - fadeProgress;
            }
            i++;
        }
    }
}

void PopUpStuff::draw() {
    for (auto sprite : sprites) {
        if (sprite && sprite->visible) {
            sprite->draw();
        }
    }
}

void PopUpStuff::clear() {
    for (auto sprite : sprites) {
        if (sprite) {
            delete sprite;
        }
    }
    sprites.clear();
    timers.clear();
}