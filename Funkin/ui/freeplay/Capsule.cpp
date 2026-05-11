#include "Capsule.h"
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <flixel/animation/FlxAnimationController.h>
#include <flixel/text/FlxText.h>
#include <flixel/FlxG.h>
#include <flixel/math/FlxMath.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

inline SDL_Color HexToSDLColor(uint32_t hex) {
    return SDL_Color{
        static_cast<Uint8>((hex >> 16) & 0xFF),
        static_cast<Uint8>((hex >> 8) & 0xFF),
        static_cast<Uint8>(hex & 0xFF),
        static_cast<Uint8>((hex >> 24) & 0xFF)
    };
}

Capsule::Capsule(const std::string& _song, const std::string& _displayName, const std::string& _icon, int _week, const std::string& _album)
    : song(_song), week(_week), album(_album), targetPos(0.0f, 0.0f), xPositionOffset(0.0f), selected(true)
{
    capsuleSprite = new flixel::FlxSprite();
    
    std::string xmlPath = ASSETS_PATH "assets/images/menu/freeplay/freeplayCapsule.xml";
    std::string pngPath = ASSETS_PATH "assets/images/menu/freeplay/freeplayCapsule.png";
    
    std::ifstream xmlFile(xmlPath);
    std::stringstream buffer;
    buffer << xmlFile.rdbuf();
    std::string xmlContent = buffer.str();
    
    auto atlasFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(pngPath.c_str(), xmlContent);
    if (atlasFrames) {
        capsuleSprite->frames = atlasFrames;
        capsuleSprite->texture = atlasFrames->texture;
        capsuleSprite->ownsTexture = false;
        capsuleSprite->animation = new flixel::animation::FlxAnimationController();
        
        auto selectedIndices = atlasFrames->getFramesByPrefix("mp3 capsule w backing SELECTED");
        auto deselectedIndices = atlasFrames->getFramesByPrefix("mp3 capsule w backing NOT SELECTED");
        
        capsuleSprite->animation->addByPrefix("selected", selectedIndices, 24, true);
        capsuleSprite->animation->addByPrefix("deselected", deselectedIndices, 24, true);
        
        capsuleSprite->animation->frameCallback = [this](const std::string& name, int frameNumber, int frameIndex) {
            if (name == "deselected") {
                capsuleSprite->offsetX = -4.0f;
                capsuleSprite->offsetY = 0.0f;
            } else {
                capsuleSprite->offsetX = 0.0f;
                capsuleSprite->offsetY = 0.0f;
            }
        };
        
        capsuleSprite->originX = 0.0f;
        capsuleSprite->originY = 0.0f;
        capsuleSprite->setScale(capsuleScale, capsuleScale);
        capsuleSprite->alpha = 1.0f;
        capsuleSprite->visible = true;
        capsuleSprite->scrollFactor.x = 0.0f;
        capsuleSprite->scrollFactor.y = 0.0f;
    }
    
    text = new flixel::FlxText(95.0f, 34.0f, 0, _displayName.c_str());
    text->setFont(ASSETS_PATH "assets/fonts/5by7.ttf");
    text->setSize(32);
    text->alpha = 1.0f;
    text->scrollFactor.x = 0.0f;
    text->scrollFactor.y = 0.0f;
    text->setColor(selectColor);
    text->setBorderStyle(flixel::FlxTextBorderStyle::OUTLINE, HexToSDLColor(selectBorderColor), 1.0f);
    icon = new flixel::FlxSprite(0.0f, 0.0f);
    std::string iconXmlPath = ASSETS_PATH "assets/images/menu/freeplay/icons/" + _icon + ".xml";
    std::string iconPngPath = ASSETS_PATH "assets/images/menu/freeplay/icons/" + _icon + ".png";
    
    std::ifstream iconXmlFile(iconXmlPath);
    std::stringstream iconBuffer;
    iconBuffer << iconXmlFile.rdbuf();
    std::string iconXmlContent = iconBuffer.str();
    
    auto iconFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(iconPngPath.c_str(), iconXmlContent);
    if (iconFrames) {
        icon->frames = iconFrames;
        icon->texture = iconFrames->texture;
        icon->ownsTexture = false;
        
        icon->animation = new flixel::animation::FlxAnimationController();
        
        auto idleIndices = iconFrames->getFramesByPrefix("idle");
        auto confirmIndices = iconFrames->getFramesByPrefix("confirm0");
        
        icon->animation->addByPrefix("idle", idleIndices, 0, false);
        icon->animation->addByPrefix("confirm", confirmIndices, 12, false);
        icon->animation->play("idle", true);
        
        icon->originX = 0.0f;
        icon->originY = 0.0f;
        icon->setScale(2.0f, 2.0f);
        icon->alpha = 1.0f;
        icon->visible = true;
        icon->scrollFactor.x = 0.0f;
        icon->scrollFactor.y = 0.0f;
    }
    
    deselect();
}

Capsule::~Capsule() {
    if (capsuleSprite) delete capsuleSprite;
    if (icon) delete icon;
    if (text) delete text;
}

void Capsule::confirm() {
    if (icon && icon->animation) {
        icon->animation->play("confirm", true);
    }
}

void Capsule::update(float elapsed) {
    const float fpsLerp = 1.0f - std::pow(1.0f - 0.3f, elapsed * 60.0f);
    x = flixel::math::lerp(x, targetPos.x, fpsLerp) + xPositionOffset;
    
    const float fpsLerpY = 1.0f - std::pow(1.0f - 0.4f, elapsed * 60.0f);
    y = flixel::math::lerp(y, targetPos.y, fpsLerpY);
    
    if (capsuleSprite) {
        capsuleSprite->x = x;
        capsuleSprite->y = y;
        capsuleSprite->update(elapsed);
        if (capsuleSprite->animation) capsuleSprite->animation->update(elapsed);
    }
    
    if (text) {
        text->x = x + 95.0f;
        text->y = y + 34.0f;
        text->update(elapsed);
    }
    
    if (icon) {
        icon->x = x;
        icon->y = y;
        icon->update(elapsed);
        if (icon->animation) icon->animation->update(elapsed);
    }

    flixel::FlxSprite::update(elapsed);
}

void Capsule::select() {
    if (selected) return;
    
    if (capsuleSprite && capsuleSprite->animation) {
        capsuleSprite->animation->play("selected", true);
    }
    
    if (text) {
        text->setColor(selectColor);
        text->setBorderStyle(flixel::FlxTextBorderStyle::OUTLINE, HexToSDLColor(selectBorderColor), 1.0f);
    }
    
    selected = true;
}

void Capsule::deselect() {
    if (!selected) return;
    
    if (capsuleSprite && capsuleSprite->animation) {
        capsuleSprite->animation->play("deselected", true);
    }
    
    if (text) {
        text->setColor(deselectColor);
        text->setBorderStyle(flixel::FlxTextBorderStyle::OUTLINE, HexToSDLColor(deselectBorderColor), 1.0f);
    }
    
    selected = false;
}

void Capsule::snapToTargetPos() {
    x = targetPos.x + xPositionOffset;
    y = targetPos.y;
    
    if (capsuleSprite) {
        capsuleSprite->x = x;
        capsuleSprite->y = y;
    }
    if (text) {
        text->x = x + 95.0f;
        text->y = y + 34.0f;
    }
    if (icon) {
        icon->x = x;
        icon->y = y;
    }
}

float Capsule::intendedX(int index) {
    return (270.0f + (60.0f * std::sin(static_cast<float>(index + 1)))) + 80.0f;
}

float Capsule::intendedY(int index) {
    return (((index + 1) * (capsuleHeight * capsuleScale + 10.0f)) + 120.0f) + 18.0f - (index < -1 ? 100.0f : 0.0f);
}