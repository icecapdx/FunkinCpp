#include "Alphabet.h"
#include <flixel/FlxG.h>
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <SDL.h>
#include <cctype>
#include <fstream>
#include <sstream>

static flixel::graphics::frames::FlxAtlasFrames* alphabetFrames = nullptr;

Alphabet::Alphabet(const std::string& text, int x, int y) 
    : baseX(x)
    , baseY(y)
    , currentText(text)
    , camera(nullptr)
    , isMenuItem(false)
    , targetY(0)
    , alpha(1.0f)
{
    if (!alphabetFrames) {
        std::ifstream file(ASSETS_PATH "assets/images/genUI/alphabet.xml");
        std::string xmlText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        alphabetFrames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(
            ASSETS_PATH "assets/images/genUI/alphabet.png",
            xmlText
        );
    }
    
    createLetters(text);
}

Alphabet::~Alphabet() {
    clearLetters();
}

void Alphabet::clearLetters() {
    for (auto* spr : letters) {
        if (spr) {
            delete spr;
        }
    }
    letters.clear();
}

void Alphabet::createLetters(const std::string& text) {
    int curX = baseX;
    int curY = baseY;
    const int letterSpacing = 48;
    const int spaceWidth = 48;
    const int lineHeight = 60;
    
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        
        if (c == '\n') {
            curX = baseX;
            curY += lineHeight;
            continue;
        }
        
        if (c == ' ') {
            letters.push_back(nullptr);
            curX += spaceWidth;
            continue;
        }
        
        flixel::FlxSprite* spr = new flixel::FlxSprite(curX, curY);
        spr->frames = alphabetFrames;
        spr->texture = alphabetFrames->texture;
        spr->ownsTexture = false;
        spr->animation = new flixel::animation::FlxAnimationController();
        
        spr->scrollFactor.x = 0.0f;
        spr->scrollFactor.y = 0.0f;
        
        std::string animName;
        if (std::isdigit(c)) {
            animName = std::string(1, c);
        } else if (std::isalpha(c)) {
            animName = std::string(1, c);
            if (std::isupper(c)) {
                animName += " bold";
            } else {
                animName += " lowercase";
            }
        } else {
            animName = std::string(1, c);
        }
        
        auto frameIndices = alphabetFrames->getFramesByPrefix(animName);
        if (!frameIndices.empty()) {
            spr->animation->addByPrefix(animName, frameIndices, 24, true);
            spr->animation->play(animName);
            spr->updateHitbox();
            letters.push_back(spr);
            curX += letterSpacing;
        } else {
            delete spr;
            letters.push_back(nullptr);
            curX += letterSpacing;
        }
    }
}

void Alphabet::setText(const std::string& newText) {
    if (currentText == newText) return;
    
    currentText = newText;
    clearLetters();
    createLetters(newText);
}

void Alphabet::screenCenter() {
    int totalWidth = static_cast<int>(letters.size()) * 48;
    int startX = (flixel::FlxG::width / 2) - (totalWidth / 2);
    int curX = startX;
    int y = baseY;
    
    for (auto* spr : letters) {
        if (spr) {
            spr->x = curX;
            spr->y = y;
            curX += 48;
        } else {
            curX += 48;
        }
    }
}

void Alphabet::setAlpha(float alpha) {
    this->alpha = alpha;
    for (auto* spr : letters) {
        if (spr) {
            spr->alpha = alpha;
        }
    }
}

void Alphabet::setVisible(bool visible) {
    for (auto* spr : letters) {
        if (spr) {
            spr->visible = visible;
        }
    }
}

void Alphabet::setScale(float scaleX, float scaleY) {
    for (auto* spr : letters) {
        if (spr) {
            spr->scale.x = scaleX;
            spr->scale.y = scaleY;
        }
    }
}

void Alphabet::setCamera(flixel::FlxCamera* cam) {
    camera = cam;
    for (auto* spr : letters) {
        if (spr) {
            spr->camera = cam;
        }
    }
}

void Alphabet::update(float elapsed) {
    if (isMenuItem) {
        float scaledY = targetY;
        float targetYPos = (scaledY * 120.0f) + (flixel::FlxG::height * 0.48f);
        float targetXPos = (targetY * 20.0f) + 90.0f;
        
        baseY += static_cast<int>((targetYPos - baseY) * 0.16f);
        baseX += static_cast<int>((targetXPos - baseX) * 0.16f);
        
        int curX = baseX;
        int curY = baseY;
        const int letterSpacing = 48;
        const int spaceWidth = 48;
        
        for (auto* spr : letters) {
            if (spr) {
                spr->y = curY;
                spr->x = curX;
                curX += letterSpacing;
            } else {
                curX += spaceWidth;
            }
        }
    }
    
    for (auto* spr : letters) {
        if (spr) {
            spr->alpha = this->alpha;
            spr->visible = true;
            spr->update(elapsed);
            if (spr->animation) {
                spr->animation->update(elapsed);
            }
        }
    }
}

void Alphabet::draw() {
    for (auto* spr : letters) {
        if (spr && spr->visible) {
            spr->draw();
        }
    }
    
    if (!letters.empty() && letters[0] && letters[0]->texture) {
        SDL_SetTextureAlphaMod(letters[0]->texture, 255);
    }
}