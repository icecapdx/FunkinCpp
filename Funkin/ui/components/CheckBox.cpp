#include "CheckBox.h"
#include <flixel/FlxG.h>
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <fstream>
#include <sstream>
#include <iostream>

CheckBox::CheckBox(float x, float y, bool checked)
    : FlxSprite(x, y)
    , checked(false)
{
    visible = true;
    
    std::ifstream file(ASSETS_PATH "assets/images/genUI/checkboxThingie.xml");
    std::string xmlText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    auto checkboxFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(
        ASSETS_PATH "assets/images/genUI/checkboxThingie.png",
        xmlText
    );
    
    frames = checkboxFrames;
    texture = checkboxFrames->texture;
    ownsTexture = false;
    animation = new flixel::animation::FlxAnimationController();
    
    auto staticFrames = checkboxFrames->getFramesByPrefix("Check Box unselected");
    auto checkedFrames = checkboxFrames->getFramesByPrefix("Check Box selecting animation");
        
    if (!staticFrames.empty()) {
        animation->addByPrefix("static", staticFrames, 24, false);
    }
    if (!checkedFrames.empty()) {
        animation->addByPrefix("checked", checkedFrames, 24, false);
    }
    
    setChecked(checked);
    
    if (frames && animation && !animation->current.empty()) {
        int frameIndex = animation->getCurrentFrame();
        if (frameIndex >= 0 && frameIndex < static_cast<int>(frames->frames.size())) {
            const auto& frame = frames->frames[frameIndex];
            frameWidth = frame.sourceSize.w;
            frameHeight = frame.sourceSize.h;
            sourceRect.w = frame.rect.w;
            sourceRect.h = frame.rect.h;
            sourceRect.x = frame.rect.x;
            sourceRect.y = frame.rect.y;
        }
    }
    
    scale.x = 0.7f;
    scale.y = 0.7f;
    scaleX = 0.7f;
    scaleY = 0.7f;
    updateHitbox();
}

CheckBox::~CheckBox() {
}

void CheckBox::draw() {
    float camScrollX = 0.0f;
    float camScrollY = 0.0f;
    float camZoom = 1.0f;
    if (camera) {
        camScrollX = camera->scroll.x * scrollFactor.x;
        camScrollY = camera->scroll.y * scrollFactor.y;
        camZoom = camera->zoom;
    }
    
    float finalX = (x - offsetX - camScrollX) * camZoom;
    float finalY = (y - offsetY - camScrollY) * camZoom;
    FlxSprite::draw();
}

void CheckBox::setChecked(bool value) {
    checked = value;
    if (animation) {
        if (checked) {
            animation->play("checked", true);
        } else {
            animation->play("static");
        }
    }
}

void CheckBox::update(float elapsed) {
    FlxSprite::update(elapsed);
    
    if (animation) {
        animation->update(elapsed);
    }
    
    if (frames && animation && !frames->frames.empty()) {
        int frameIdx = animation->getCurrentFrame();
        if (frameIdx >= 0 && frameIdx < static_cast<int>(frames->frames.size())) {
            const auto& frame = frames->frames[frameIdx];
            frameWidth = frame.sourceSize.w;
            frameHeight = frame.sourceSize.h;
            sourceRect.w = frame.rect.w;
            sourceRect.h = frame.rect.h;
            sourceRect.x = frame.rect.x;
            sourceRect.y = frame.rect.y;
            width = static_cast<float>(frameWidth);
            height = static_cast<float>(frameHeight);
        }
    }
    
    if (animation && !animation->current.empty()) {
        if (animation->current == "static") {
            offsetX = 0;
            offsetY = 0;
        } else if (animation->current == "checked") {
            offsetX = 17;
            offsetY = 70;
        }
    }
}