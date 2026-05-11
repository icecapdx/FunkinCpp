#include "Stage.h"
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Stage::Stage(const std::string& stageName) {
    curStage = stageName;
    defaultZoom = 1.05f;
    buildStage();
}

Stage::~Stage() {
    for (auto sprite : sprites) {
        if (sprite) {
            delete sprite;
        }
    }
    sprites.clear();
}

bool Stage::isPixelStage() const {
    return curStage == "school" || curStage == "schoolEvil";
}

std::string Stage::getStageFromSong(const std::string& songName) {
    std::string lowerSong = songName;
    std::transform(lowerSong.begin(), lowerSong.end(), lowerSong.begin(), ::tolower);
    
    if (lowerSong == "spookeez" || lowerSong == "monster" || lowerSong == "south") {
        return "spooky";
    }
    else if (lowerSong == "pico" || lowerSong == "blammed" || lowerSong == "philly") {
        return "philly";
    }
    else if (lowerSong == "milf" || lowerSong == "satin-panties" || lowerSong == "high") {
        return "limo";
    }
    else if (lowerSong == "cocoa" || lowerSong == "eggnog") {
        return "mall";
    }
    else if (lowerSong == "winter-horrorland") {
        return "mallEvil";
    }
    else if (lowerSong == "senpai" || lowerSong == "roses") {
        return "school";
    }
    else if (lowerSong == "thorns") {
        return "schoolEvil";
    }
    return "stage";
}

void Stage::buildStage() {
    if (loadFromJSON(curStage)) {
        return;
    }
    
    if (curStage == "stage") {
        buildDefaultStage();
    }
    else {
        buildDefaultStage();
    }
}

bool Stage::loadFromJSON(const std::string& stageName) {
    std::string jsonPath = ASSETS_PATH "assets/data/stages/" + stageName + ".json";
    std::ifstream file(jsonPath);
    
    if (!file.is_open()) {
        return false;
    }
    
    try {
        json stageData;
        file >> stageData;
        file.close();
        
        defaultZoom = stageData.value("defaultZoom", 1.05f);
        
        if (stageData.contains("sprites")) {
            for (const auto& spriteData : stageData["sprites"]) {
                std::string type = spriteData.value("type", "static");
                std::string imagePath = spriteData.value("imagePath", "");
                
                if (imagePath.empty()) continue;
                
                float x = 0.0f, y = 0.0f;
                if (spriteData.contains("position") && spriteData["position"].is_array() && spriteData["position"].size() >= 2) {
                    x = spriteData["position"][0].get<float>();
                    y = spriteData["position"][1].get<float>();
                }
                
                flixel::FlxSprite* sprite = nullptr;
                
                if (type == "animated" && spriteData.contains("xmlPath")) {
                    std::string xmlPath = spriteData["xmlPath"].get<std::string>();
                    sprite = createAnimatedSprite(x, y, imagePath, xmlPath);
                    
                    if (spriteData.contains("animations")) {
                        for (const auto& anim : spriteData["animations"]) {
                            std::string animName = anim.value("name", "");
                            std::string prefix = anim.value("prefix", "");
                            int frameRate = anim.value("frameRate", 24);
                            bool loop = anim.value("loop", false);
                            
                            if (!animName.empty() && !prefix.empty()) {
                                auto frames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(imagePath, xmlPath);
                                auto animFrames = frames->getFramesByPrefix(prefix);
                                if (!animFrames.empty()) {
                                    sprite->animation->addByPrefix(animName, animFrames, frameRate, loop);
                                }
                            }
                        }
                        
                        std::string startAnim = spriteData.value("startingAnimation", "");
                        if (!startAnim.empty() && sprite->animation) {
                            sprite->animation->play(startAnim);
                        }
                    }
                } else {
                    sprite = createSprite(x, y, imagePath);
                }
                
                if (sprite) {
                    if (spriteData.contains("scrollFactor") && spriteData["scrollFactor"].is_array() && spriteData["scrollFactor"].size() >= 2) {
                        float scrollX = spriteData["scrollFactor"][0].get<float>();
                        float scrollY = spriteData["scrollFactor"][1].get<float>();
                        sprite->scrollFactor.set(scrollX, scrollY);
                    }
                    
                    if (spriteData.contains("scale") && spriteData["scale"].is_array() && spriteData["scale"].size() >= 2) {
                        float scaleX = spriteData["scale"][0].get<float>();
                        float scaleY = spriteData["scale"][1].get<float>();
                        sprite->setScale(scaleX, scaleY);
                    }
                    
                    if (spriteData.value("updateHitbox", false)) {
                        sprite->updateHitbox();
                    }
                    
                    if (spriteData.contains("active")) {
                        sprite->active = spriteData["active"].get<bool>();
                    }
                    
                    if (spriteData.contains("visible")) {
                        sprite->visible = spriteData["visible"].get<bool>();
                    }
                }
            }
        }
        
        std::cout << "Loaded stage from JSON: " << stageName << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing stage JSON for " << stageName << ": " << e.what() << std::endl;
        file.close();
        return false;
    }
}

flixel::FlxSprite* Stage::createSprite(float x, float y, const std::string& imagePath) {
    flixel::FlxSprite* sprite = new flixel::FlxSprite(x, y);
    sprite->loadGraphic(imagePath);
    sprites.push_back(sprite);
    return sprite;
}

flixel::FlxSprite* Stage::createAnimatedSprite(float x, float y, const std::string& imagePath, const std::string& xmlPath) {
    flixel::FlxSprite* sprite = new flixel::FlxSprite(x, y);
    auto frames = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(imagePath, xmlPath);
    sprite->frames = frames;
    sprite->texture = frames->texture;
    sprite->ownsTexture = false;
    sprite->animation = new flixel::animation::FlxAnimationController();
    sprites.push_back(sprite);
    return sprite;
}

void Stage::buildDefaultStage() {
    defaultZoom = 1.05f;
    
    auto bg = createSprite(-600, -200, ASSETS_PATH "assets/images/stages/stage/stageback.png");
    bg->scrollFactor.set(0.9f, 0.9f);
    bg->active = false;
    
    auto stageFront = createSprite(-650, 600, ASSETS_PATH "assets/images/stages/stage/stagefront.png");
    stageFront->setScale(1.1f, 1.1f);
    stageFront->updateHitbox();
    stageFront->scrollFactor.set(0.9f, 0.9f);
    stageFront->active = false;
    
    auto stageCurtains = createSprite(-500, -300, ASSETS_PATH "assets/images/stages/stage/stagecurtains.png");
    stageCurtains->setScale(0.9f, 0.9f);
    stageCurtains->updateHitbox();
    stageCurtains->scrollFactor.set(1.3f, 1.3f);
    stageCurtains->active = false;
}