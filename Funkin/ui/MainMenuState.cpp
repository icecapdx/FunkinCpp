#include "MainMenuState.h"
#include "StoryMenuState.h"
#include "TitleState.h"
#include "NewFreeplayState.h"
#include "OptionsState.h"
#include "CreditsState.h"
#include "../play/PlayState.h"
#include "../game/GameConfig.h"
#include "../play/input/Controls.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/sound/FlxSound.h>
#include <flixel/effects/FlxFlicker.h>
#include <flixel/util/FlxTimer.h>
#include <flixel/util/FlxColor.h>
#include <flixel/tweens/FlxTween.h>
#include <SDL_mixer.h>
#include <iostream>

MainMenuState::MainMenuState() 
    : bg(nullptr)
    , magenta(nullptr)
    , camFollow(nullptr)
    , selectedIndex(0)
    , canSelect(true)
    , exiting(false)
{
}

MainMenuState::~MainMenuState() {
    destroy();
}

void MainMenuState::create() {
    if (!flixel::FlxG::camera) {
        flixel::FlxG::camera = new flixel::FlxCamera(0.0f, 0.0f, 0, 0, 1.0f);
    }
    
    bg = new flixel::FlxSprite(-80, 0);
    bg->loadGraphic(ASSETS_PATH "assets/images/menuBG.png");
    bg->scrollFactor.x = 0.0f;
    bg->scrollFactor.y = 0.18f;
    bg->setGraphicSize(static_cast<int>(bg->width * 1.18f));
    bg->updateHitbox();
    bg->screenCenter();
    bg->camera = flixel::FlxG::camera;
    
    camFollow = new flixel::FlxObject(0, 0, 1, 1);
    
    magenta = new flixel::FlxSprite(-80, 0);
    magenta->loadGraphic(ASSETS_PATH "assets/images/menuBGMagenta.png");
    magenta->scrollFactor.x = 0.0f;
    magenta->scrollFactor.y = 0.18f;
    magenta->setGraphicSize(static_cast<int>(magenta->width * 1.18f));
    magenta->updateHitbox();
    magenta->screenCenter();
    magenta->visible = false;
    magenta->camera = flixel::FlxG::camera;
    
    createMenuItem("storymode", ASSETS_PATH "assets/images/menu/mainmenu/storymode", []() {
        flixel::FlxG::game->switchState(new StoryMenuState());
    });
    
    createMenuItem("freeplay", ASSETS_PATH "assets/images/menu/mainmenu/freeplay", [this]() {
        if (camFollow) {
            flixel::FlxPoint camPos(camFollow->x, camFollow->y);
            flixel::FlxG::game->switchState(new NewFreeplayState(true, camPos));
        } else {
            flixel::FlxG::game->switchState(new NewFreeplayState(true, flixel::FlxPoint(0.0f, 0.0f)));
        }
    });
    
    createMenuItem("options", ASSETS_PATH "assets/images/menu/mainmenu/options", []() {
        flixel::FlxG::game->switchState(new OptionsState());
    });
    
    createMenuItem("credits", ASSETS_PATH "assets/images/menu/mainmenu/credits", []() {
        flixel::FlxG::game->switchState(new CreditsState());
    });
    
    int spacing = 160;
    int top = (flixel::FlxG::height - (spacing * (static_cast<int>(menuItems.size()) - 1))) / 2;
    for (size_t i = 0; i < menuItems.size(); i++) {
        menuItems[i]->x = flixel::FlxG::width / 2.0f;
        menuItems[i]->y = static_cast<float>(top + spacing * i);
        menuItems[i]->scrollFactor.x = 0.0f;
        menuItems[i]->scrollFactor.y = 0.4f;
        menuItems[i]->camera = flixel::FlxG::camera;
        
        if (menuItems[i]->centered && menuItems[i]->frames) {
            menuItems[i]->x -= menuItems[i]->width / 2.0f;
        }
    }
    
    if (flixel::FlxG::camera && camFollow) {
        flixel::FlxG::camera->target = camFollow;
        flixel::FlxG::camera->followLerp = 0.06f;
        flixel::FlxG::camera->deadzone = nullptr;
        flixel::FlxG::camera->focusOn(flixel::FlxPoint(camFollow->x, camFollow->y));
    }
    
    if (!Mix_PlayingMusic()) {
        flixel::FlxG::sound.playMusic(ASSETS_PATH "assets/music/freakyMenu.ogg", 0.0f, true);
    }
    
    onMenuItemChange();
    
    startTransitionIn(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, -1));
}

void MainMenuState::update(float elapsed) {
    FunkinState::update(elapsed);
    
    if (bg) bg->update(elapsed);
    if (magenta) magenta->update(elapsed);
    if (camFollow) camFollow->update(elapsed);
    
    for (auto* item : menuItems) {
        if (item) {
            item->update(elapsed);
            
            if (item->centered) {
                item->x = flixel::FlxG::width / 2.0f - item->width / 2.0f;
            }
        }
    }
    
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(menuItems.size()) && camFollow) {
        camFollow->x = menuItems[selectedIndex]->x + menuItems[selectedIndex]->width / 2.0f;
        camFollow->y = menuItems[selectedIndex]->y + menuItems[selectedIndex]->height / 2.0f;
    }
    
    if (flixel::FlxG::camera) {
        if (flixel::FlxG::camera->scroll.y < 0.0f) {
            flixel::FlxG::camera->scroll.y = 0.0f;
        }
    }
    
    if (flixel::FlxG::sound.music && Mix_PlayingMusic()) {
        if (Mix_VolumeMusic(-1) < static_cast<int>(1.0f * MIX_MAX_VOLUME)) {
            int currentVolume = Mix_VolumeMusic(-1);
            int targetVolume = std::min(currentVolume + static_cast<int>(0.5f * elapsed * MIX_MAX_VOLUME), MIX_MAX_VOLUME);
            Mix_VolumeMusic(targetVolume);
        }
    }
    
    Controls* controls = GameConfig::getInstance()->controls;
    
    if (canSelect && !exiting) {
        if (controls->justPressedAction(ControlAction::UI_UP)) {
            changeSelection(-1);
        }
        if (controls->justPressedAction(ControlAction::UI_DOWN)) {
            changeSelection(1);
        }
        
        if (controls->justPressedAction(ControlAction::ACCEPT)) {
            selectItem();
        }
        
        if (controls->justPressedAction(ControlAction::BACK)) {
            flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
            exiting = true;
            canSelect = false;
            
            startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
                flixel::FlxG::game->switchState(new TitleState(true));
            });
        }
    }
}

void MainMenuState::draw() {
    if (bg) bg->draw();
    if (magenta && magenta->visible) magenta->draw();
    
    for (auto* item : menuItems) {
        if (item) {
            item->draw();
        }
    }
    
    FunkinState::draw();
}

void MainMenuState::destroy() {
    if (magenta && flixel::effects::FlxFlicker::isFlickering(magenta)) {
        flixel::effects::FlxFlicker::stopFlickering(magenta);
        magenta->visible = false;
    }
    
    for (auto* item : menuItems) {
        if (item && flixel::effects::FlxFlicker::isFlickering(item)) {
            flixel::effects::FlxFlicker::stopFlickering(item);
        }
    }
    
    for (auto* item : menuItems) {
        if (item) {
            delete item;
        }
    }
    menuItems.clear();
    
    if (bg) {
        delete bg;
        bg = nullptr;
    }
    if (magenta) {
        delete magenta;
        magenta = nullptr;
    }
    if (camFollow) {
        delete camFollow;
        camFollow = nullptr;
    }
    
    FunkinState::destroy();
}

void MainMenuState::beatHit() {
}

void MainMenuState::createMenuItem(const std::string& name, const std::string& atlas, std::function<void()> callback) {
    AtlasMenuItem* item = new AtlasMenuItem(name, atlas, callback);
    item->ID = static_cast<int>(menuItems.size());
    item->centered = true;
    item->changeAnim("idle");
    menuItems.push_back(item);
}

void MainMenuState::changeSelection(int change) {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
    
    selectedIndex += change;
    
    if (selectedIndex < 0) {
        selectedIndex = static_cast<int>(menuItems.size()) - 1;
    }
    if (selectedIndex >= static_cast<int>(menuItems.size())) {
        selectedIndex = 0;
    }
    
    onMenuItemChange();
}

void MainMenuState::selectItem() {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(menuItems.size())) {
        flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
        
        canSelect = false;
        
        magenta->visible = true;
        flixel::effects::FlxFlicker::flicker(magenta, 1.1f, 0.15f, false, true);
        
        auto* selectedItem = menuItems[selectedIndex];
        flixel::effects::FlxFlicker::flicker(
            selectedItem,
            1.0f,
            0.06f,
            false,
            false,
            [selectedItem](flixel::effects::FlxFlicker* flick) {
                if (selectedItem->callback) {
                    selectedItem->callback();
                }
            }
        );
        
        for (size_t i = 0; i < menuItems.size(); i++) {
            if (static_cast<int>(i) != selectedItem->ID) {
                flixel::tweens::tween(menuItems[i], {{"alpha", 0.0f}}, 0.4f, flixel::tweens::FlxEase::quadOut);
            }
        }
    }
}

void MainMenuState::onMenuItemChange() {
    for (size_t i = 0; i < menuItems.size(); i++) {
        if (static_cast<int>(i) == selectedIndex) {
            menuItems[i]->changeAnim("selected");
        } else {
            menuItems[i]->changeAnim("idle");
        }
        
        if (menuItems[i]->centered) {
            menuItems[i]->x = flixel::FlxG::width / 2.0f - menuItems[i]->width / 2.0f;
        }
    }
    
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(menuItems.size()) && camFollow) {
        camFollow->x = menuItems[selectedIndex]->x + menuItems[selectedIndex]->width / 2.0f;
        camFollow->y = menuItems[selectedIndex]->y + menuItems[selectedIndex]->height / 2.0f;
    }
}

void MainMenuState::startExitState(std::function<void()> callback) {
    exiting = true;
    canSelect = false;
    
    if (callback) {
        callback();
    }
}
