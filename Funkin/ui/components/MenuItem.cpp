#include "MenuItem.h"
#include <flixel/FlxG.h>
#include <flixel/math/FlxMath.h>
#include <flixel/effects/FlxFlicker.h>

MenuItem::MenuItem(float x, float y, const std::string& weekFileName)
    : FlxSprite(x, y)
    , targetY(0.0f)
{
    std::string imagePath = ASSETS_PATH "assets/images/menu/story/weeks/" + weekFileName + ".png";
    loadGraphic(imagePath.c_str());
}

MenuItem::~MenuItem() {
}

void MenuItem::startFlashing() {
    flixel::effects::FlxFlicker::flicker(this, 1.0f, 0.06f, true, false);
}

void MenuItem::update(float elapsed) {
    FlxSprite::update(elapsed);
    
    if (animation) {
        animation->update(elapsed);
    }

    y = flixel::math::lerp(y, (targetY * 120.0f) + 480.0f, 0.17f);
}