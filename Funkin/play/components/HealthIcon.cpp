#include "HealthIcon.h"
#include <flixel/FlxG.h>
#include <iostream>

HealthIcon::HealthIcon(const std::string& character, bool isPlayer)
    : FlxSprite(), isPlayer(isPlayer), sprTracker(nullptr), curCharacter(""), fullIconWidth(0)
{
    iconOffsets[0] = 0.0f;
    iconOffsets[1] = 0.0f;
    scrollFactor.x = 0.0f;
    scrollFactor.y = 0.0f;
    changeIcon(character);
}

HealthIcon::~HealthIcon() {
}

void HealthIcon::update(float elapsed) {
    FlxSprite::update(elapsed);
    
    if (sprTracker != nullptr) {
        setPosition(sprTracker->x + sprTracker->width + 12.0f, sprTracker->y - 30.0f);
    }
}

void HealthIcon::changeIcon(const std::string& character) {
    if (curCharacter == character) {
        return;
    }
    
    curCharacter = character;
    
    std::string iconPath = ASSETS_PATH "assets/images/play/icons/icon-" + character + ".png";
    loadGraphic(iconPath);
    
    if (!texture) {
        iconPath = ASSETS_PATH "assets/images/play/icons/icon-face.png";
        loadGraphic(iconPath);
    }
    
    if (texture) {
        fullIconWidth = sourceRect.w;
        int iconWidth = sourceRect.w / 2;
        sourceRect.w = iconWidth;
        sourceRect.x = 0;
        
        iconOffsets[0] = (iconWidth - 150.0f) / 2.0f;
        iconOffsets[1] = (sourceRect.h - 150.0f) / 2.0f;
        
        offsetX = iconOffsets[0];
        offsetY = iconOffsets[1];
        
        if (isPlayer) {
            flipX = true;
        }
    }
}
