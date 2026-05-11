#include "DigitDisplay.h"
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <flixel/animation/FlxAnimationController.h>
#include <flixel/math/FlxMath.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iostream>

static const std::vector<std::string> numberStringDigital = {"ZERO DIGITAL", "ONE DIGITAL", "TWO DIGITAL", "THREE DIGITAL", "FOUR DIGITAL", "FIVE DIGITAL", "SIX DIGITAL", "SEVEN DIGITAL", "EIGHT DIGITAL", "NINE DIGITAL"};
static const std::vector<std::string> numberStringSimple = {"ZERO ", "ONE ", "TWO ", "THREE ", "FOUR ", "FIVE ", "SIX ", "SEVEN ", "EIGHT ", "NINE "};

Digit::Digit(float _x, float _y, const std::string& _path) : flixel::FlxSprite(_x, _y) {
    std::string xmlPath = ASSETS_PATH "assets/images/" + _path + ".xml";
    std::string pngPath = ASSETS_PATH "assets/images/" + _path + ".png";
    
    std::ifstream xmlFile(xmlPath);
    std::stringstream buffer;
    buffer << xmlFile.rdbuf();
    std::string xmlContent = buffer.str();
    
    auto atlasFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(pngPath.c_str(), xmlContent);
    if (atlasFrames) {
        frames = atlasFrames;
        texture = atlasFrames->texture;
        
        animation = new flixel::animation::FlxAnimationController();
        
        auto testDigitalIndices = atlasFrames->getFramesByPrefix("ZERO DIGITAL");
        bool useDigitalSuffix = !testDigitalIndices.empty();
        
        const std::vector<std::string>& numberString = useDigitalSuffix ? numberStringDigital : numberStringSimple;
        
        for (int i = 0; i < 10; i++) {
            auto indices = atlasFrames->getFramesByPrefix(numberString[i]);
            animation->addByPrefix(std::to_string(i), indices, 24, false);
            offsetMap[std::to_string(i)] = 0.0f;
        }
        
        animation->play("0", true);
        
        if (animation->currentFrame >= 0 && animation->currentFrame < static_cast<int>(frames->frames.size())) {
            auto& frame = frames->frames[animation->currentFrame];
            frameWidth = frame.sourceSize.w;
            frameHeight = frame.sourceSize.h;
            width = static_cast<float>(frameWidth);
            height = static_cast<float>(frameHeight);
        }
        
        auto framesPtr = frames;
        animation->frameCallback = [this, framesPtr](const std::string& name, int frameNum, int frameIndex) {
            if (frameIndex >= 0 && frameIndex < static_cast<int>(framesPtr->frames.size())) {
                auto& frame = framesPtr->frames[frameIndex];
                this->frameWidth = frame.sourceSize.w;
                this->frameHeight = frame.sourceSize.h;
            }
            setOffset();
        };
        
        setOffset();
    }
}

void Digit::setOffset() {
    centerOffsets();
    if (animation && animation->current != "") {
        offsetX += offsetMap[animation->current] * getScaleX();
    }
}

DigitDisplay::DigitDisplay(float _x, float _y, const std::string& _path, int _digitCount, float _scale, float _spacing, int _startingNumber, bool _hideTrailingZeroes)
    : x(_x), y(_y), scrollFactor(1.0f, 1.0f), digitCount(_digitCount), digitScale(_scale), spacing(_spacing), hideTrailingZeroes(_hideTrailingZeroes), camera(nullptr), lastX(_x), lastY(_y)
{
    targetNumber = _startingNumber;
    currentNumber = _startingNumber;
    tweenProgress = 0.0f;
    tweenDuration = 0.0f;
    isTweening = false;
    
    for (int i = 0; i < digitCount; i++) {
        Digit* digit = new Digit(0, 0, _path);
        digits.push_back(digit);
    }
    
    repositionDigits();
    setNumber(_startingNumber);
}

DigitDisplay::~DigitDisplay() {
    for (auto digit : digits) {
        delete digit;
    }
    digits.clear();
}

void DigitDisplay::repositionDigits() {
    float totalDistance = 0.0f;
    for (size_t i = 0; i < digits.size(); i++) {
        auto digit = digits[i];
        digit->setScale(digitScale, digitScale);
        digit->updateHitbox();
        digit->setOffset();
        digit->x = x + totalDistance;
        digit->y = y;
        totalDistance += digit->width + (spacing * digitScale);
    }
}

void DigitDisplay::setNumber(int number, bool forceAllDigitsToAnimate) {
    numString = std::to_string(number);
    
    while (numString.length() < static_cast<size_t>(digitCount)) {
        numString = "0" + numString;
    }
    
    if (numString.length() > static_cast<size_t>(digitCount)) {
        numString = numString.substr(numString.length() - digitCount);
    }
    
    for (size_t i = 0; i < numString.length() && i < digits.size(); i++) {
        digits[i]->visible = true;
        std::string charStr(1, numString[i]);
        
        if (digits[i]->animation->current != charStr || forceAllDigitsToAnimate) {
            digits[i]->animation->play(charStr, forceAllDigitsToAnimate);
            
            if (digits[i]->frames && digits[i]->animation) {
                int frameIdx = digits[i]->animation->getCurrentFrame();
                if (frameIdx >= 0 && frameIdx < static_cast<int>(digits[i]->frames->frames.size())) {
                    auto& frame = digits[i]->frames->frames[frameIdx];
                    digits[i]->frameWidth = frame.sourceSize.w;
                    digits[i]->frameHeight = frame.sourceSize.h;
                }
            }
        }
        
        if (hideTrailingZeroes && i != (numString.length() - 1) && numString[i] == '0') {
            if (i > 0) {
                digits[i]->visible = (numString[i - 1] != '0');
            } else {
                digits[i]->visible = false;
            }
        }
    }
    
    repositionDigits();
}

void DigitDisplay::tweenNumber(int newNumber, float tweenTime, bool force) {
    targetNumber = newNumber;
    tweenDuration = tweenTime;
    tweenProgress = 0.0f;
    isTweening = true;
}

void DigitDisplay::setDigitOffset(int number, float offset) {
    for (auto digit : digits) {
        digit->offsetMap[std::to_string(number)] = offset;
        digit->setOffset();
    }
}

void DigitDisplay::update(float elapsed) {
    if (x != lastX || y != lastY) {
        repositionDigits();
        lastX = x;
        lastY = y;
    }
    
    if (isTweening) {
        tweenProgress += elapsed;
        float t = std::min(tweenProgress / tweenDuration, 1.0f);
        
        t = t * t * (3.0f - 2.0f * t);
        
        int newValue = static_cast<int>(flixel::math::lerp(static_cast<float>(currentNumber), static_cast<float>(targetNumber), t));
        
        if (newValue != currentNumber) {
            currentNumber = newValue;
            setNumber(newValue, true);
        }
        
        if (tweenProgress >= tweenDuration) {
            isTweening = false;
            currentNumber = targetNumber;
            setNumber(currentNumber, true);
        }
    }
    
    for (auto digit : digits) {
        if (camera) digit->camera = camera;
        digit->scrollFactor.x = scrollFactor.x;
        digit->scrollFactor.y = scrollFactor.y;
        digit->update(elapsed);
    }
}

void DigitDisplay::draw() {
    for (auto digit : digits) {
        if (digit->visible) {
            digit->draw();
        }
    }
}