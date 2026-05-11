#include "MenuCharacter.h"
#include <flixel/graphics/frames/FlxAtlasFrames.h>
#include <flixel/animation/FlxAnimationController.h>
#include <fstream>
#include <sstream>

MenuCharacter::MenuCharacter(float x, const std::string& characterName)
    : FlxSprite(x, 0.0f)
    , character(characterName)
{
    std::string xmlPath = ASSETS_PATH "assets/images/menu/story/campaign_menu_UI_characters.xml";
    std::string pngPath = ASSETS_PATH "assets/images/menu/story/campaign_menu_UI_characters.png";
    
    std::ifstream xmlFile(xmlPath);
    if (xmlFile.is_open()) {
        std::stringstream buffer;
        buffer << xmlFile.rdbuf();
        std::string xmlContent = buffer.str();
        xmlFile.close();
        
        auto tex = flixel::graphics::frames::FlxAtlasFrames::fromSparrow(pngPath.c_str(), xmlContent);
        if (tex) {
            frames = tex;
            texture = tex->texture;
            ownsTexture = false;
            
            if (!animation) {
                animation = new flixel::animation::FlxAnimationController();
            }
            
            auto bfFrames = frames->getFramesByPrefix("BF idle dance white");
            auto bfConfirmFrames = frames->getFramesByPrefix("BF HEY!!");
            auto gfFrames = frames->getFramesByPrefix("GF Dancing Beat WHITE");
            auto dadFrames = frames->getFramesByPrefix("Dad idle dance BLACK LINE");
            auto spookyFrames = frames->getFramesByPrefix("spooky dance idle BLACK LINES");
            auto picoFrames = frames->getFramesByPrefix("Pico Idle Dance");
            auto momFrames = frames->getFramesByPrefix("Mom Idle BLACK LINES");
            auto parentsFrames = frames->getFramesByPrefix("Parent Christmas Idle");
            auto senpaiFrames = frames->getFramesByPrefix("SENPAI idle Black Lines");
            auto tankmanFrames = frames->getFramesByPrefix("Tankman Menu BLACK");
            
            if (!bfFrames.empty()) animation->addByPrefix("bf", bfFrames, 24, true);
            if (!bfConfirmFrames.empty()) animation->addByPrefix("bfConfirm", bfConfirmFrames, 24, false);
            if (!gfFrames.empty()) animation->addByPrefix("gf", gfFrames, 24, true);
            if (!dadFrames.empty()) animation->addByPrefix("dad", dadFrames, 24, true);
            if (!spookyFrames.empty()) animation->addByPrefix("spooky", spookyFrames, 24, true);
            if (!picoFrames.empty()) animation->addByPrefix("pico", picoFrames, 24, true);
            if (!momFrames.empty()) animation->addByPrefix("mom", momFrames, 24, true);
            if (!parentsFrames.empty()) animation->addByPrefix("parents-christmas", parentsFrames, 24, true);
            if (!senpaiFrames.empty()) animation->addByPrefix("senpai", senpaiFrames, 24, true);
            if (!tankmanFrames.empty()) animation->addByPrefix("tankman", tankmanFrames, 24, true);
            
            animation->play(character);
            
            if (frames && !frames->frames.empty()) {
                const auto& frame = frames->frames[0];
                frameWidth = frame.sourceSize.w;
                frameHeight = frame.sourceSize.h;
                width = static_cast<float>(frameWidth);
                height = static_cast<float>(frameHeight);
            }
        }
    }
}

MenuCharacter::~MenuCharacter() {
}

void MenuCharacter::update(float elapsed) {
    FlxSprite::update(elapsed);
    
    if (animation) {
        animation->update(elapsed);
    }
}