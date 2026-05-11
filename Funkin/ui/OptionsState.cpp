#include "OptionsState.h"
#include "components/CheckBox.h"
#include "MainMenuState.h"
#include "../play/input/Controls.h"
#include "../game/GameConfig.h"
#include <flixel/FlxG.h>
#include <flixel/FlxGame.h>
#include <flixel/util/FlxColor.h>
#include <SDL.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

OptionsState::OptionsState()
    : bg(nullptr)
    , controlsCamera(nullptr)
    , camFollowY(0.0f)
    , selectedIndex(0)
    , currentSubMenu(SubMenu::None)
    , prefSelectedIndex(0)
    , deviceLabel(nullptr)
    , controlSelectedIndex(0)
    , controlColumnIndex(0)
    , awaitingInput(false)
    , isGamepadMode(false)
    , selectingDevice(false)
{
}

OptionsState::~OptionsState() {
    destroy();
}

void OptionsState::create() {
    if (!flixel::FlxG::camera) {
        flixel::FlxG::camera = new flixel::FlxCamera(0.0f, 0.0f, 0, 0, 1.0f);
    }
    
    controlsCamera = new flixel::FlxCamera(0.0f, 0.0f, flixel::FlxG::width, flixel::FlxG::height, 1.0f);
    controlsCamera->bgColor = 0x00000000;
    camFollowY = 0.0f;
    
    bg = new flixel::FlxSprite(0, 0);
    bg->loadGraphic(ASSETS_PATH "assets/images/menuDesat.png");
    bg->setGraphicSize(static_cast<int>(bg->width * 1.1f));
    bg->updateHitbox();
    bg->screenCenter();
    bg->scrollFactor.x = 0.0f;
    bg->scrollFactor.y = 0.0f;
    bg->camera = flixel::FlxG::camera;
    
    menuItems.push_back(new Alphabet("PREFERENCES", 0, 200));
    menuItems.push_back(new Alphabet("CONTROLS", 0, 300));
    menuItems.push_back(new Alphabet("MODS", 0, 400));
    menuItems.push_back(new Alphabet("EXIT", 0, 500));
    
    for (auto* item : menuItems) {
        item->screenCenter();
        item->setCamera(flixel::FlxG::camera);
    }
    
    loadPreferences();
    
    std::vector<std::pair<std::string, std::string>> prefs = {
        {"downscroll", "DOWNSCROLL"},
        {"ghostTapping", "GHOST TAPPING"}
    };
    
    int yPos = 30;
    for (size_t i = 0; i < prefs.size(); i++) {
        const auto& pref = prefs[i];
        
        Alphabet* label = new Alphabet(pref.second, 120, yPos);
        prefLabels.push_back(label);
        
        CheckBox* checkbox = new CheckBox(flixel::FlxG::width - 150, yPos, preferences[pref.first]);
        checkbox->camera = flixel::FlxG::camera;
        checkbox->scrollFactor.x = 0.0f;
        checkbox->scrollFactor.y = 0.0f;
        checkBoxes.push_back(checkbox);
        
        yPos += 120;
    }
    prefLabels.push_back(new Alphabet("BACK", 120, yPos));
    
    yPos = 50;
    
#ifdef __SWITCH__
    deviceLabel = new Alphabet("GAMEPAD", 0, yPos);
    deviceLabel->screenCenter();
    isGamepadMode = true;
#else
    deviceLabel = new Alphabet("KEYBOARD", 0, yPos);
    deviceLabel->screenCenter();
#endif
    yPos += 80;
    
    Alphabet* notesHeader = new Alphabet("NOTES", 0, yPos);
    notesHeader->screenCenter();
    controlHeaders.push_back(notesHeader);
    yPos += 70;
    
    std::vector<std::pair<std::string, int>> noteControls = {
        {"LEFT", static_cast<int>(ControlAction::NOTE_LEFT)},
        {"DOWN", static_cast<int>(ControlAction::NOTE_DOWN)},
        {"UP", static_cast<int>(ControlAction::NOTE_UP)},
        {"RIGHT", static_cast<int>(ControlAction::NOTE_RIGHT)}
    };
    
    for (const auto& control : noteControls) {
        Alphabet* label = new Alphabet(control.first, 150, yPos);
        controlLabels.push_back(label);
        
        Alphabet* bindLabel = new Alphabet("---", 480, yPos);
        controlBindLabels.push_back(bindLabel);
        
        Alphabet* bindLabelAlt = new Alphabet("---", 800, yPos);
        controlBindLabelsAlt.push_back(bindLabelAlt);
        
        yPos += 70;
    }
    
    yPos += 20;
    
    Alphabet* uiHeader = new Alphabet("UI", 0, yPos);
    uiHeader->screenCenter();
    controlHeaders.push_back(uiHeader);
    yPos += 70;
    
    std::vector<std::pair<std::string, int>> uiControls = {
        {"LEFT", static_cast<int>(ControlAction::UI_LEFT)},
        {"DOWN", static_cast<int>(ControlAction::UI_DOWN)},
        {"UP", static_cast<int>(ControlAction::UI_UP)},
        {"RIGHT", static_cast<int>(ControlAction::UI_RIGHT)},
        {"ACCEPT", static_cast<int>(ControlAction::ACCEPT)},
        {"BACK", static_cast<int>(ControlAction::BACK)},
        {"PAUSE", static_cast<int>(ControlAction::PAUSE)},
        {"RESET", static_cast<int>(ControlAction::RESET)}
    };
    
    for (const auto& control : uiControls) {
        Alphabet* label = new Alphabet(control.first, 150, yPos);
        controlLabels.push_back(label);
        
        Alphabet* bindLabel = new Alphabet("---", 480, yPos);
        controlBindLabels.push_back(bindLabel);
        
        Alphabet* bindLabelAlt = new Alphabet("---", 800, yPos);
        controlBindLabelsAlt.push_back(bindLabelAlt);
        
        yPos += 70;
    }
    
    yPos += 20;
    controlLabels.push_back(new Alphabet("BACK", 120, yPos));
    controlBindLabels.push_back(new Alphabet("", 480, yPos));
    controlBindLabelsAlt.push_back(new Alphabet("", 800, yPos));
    
    if (deviceLabel) {
        deviceLabel->setCamera(controlsCamera);
        deviceLabel->setScale(0.8f, 0.8f);
    }
    for (auto* header : controlHeaders) {
        if (header) {
            header->setCamera(controlsCamera);
            header->setScale(0.75f, 0.75f);
        }
    }
    for (auto* label : controlLabels) {
        if (label) {
            label->setCamera(controlsCamera);
            label->setScale(0.7f, 0.7f);
        }
    }
    for (auto* label : controlBindLabels) {
        if (label) {
            label->setCamera(controlsCamera);
            label->setScale(0.7f, 0.7f);
        }
    }
    for (auto* label : controlBindLabelsAlt) {
        if (label) {
            label->setCamera(controlsCamera);
            label->setScale(0.7f, 0.7f);
        }
    }
    
    if (!controlLabels.empty()) {
        camFollowY = static_cast<float>(controlLabels[0]->getY());
    }
    
    updateControlBindings();
    
    startTransitionIn(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, -1));
}

void OptionsState::update(float elapsed) {
    FunkinState::update(elapsed);
    
    if (bg) bg->update(elapsed);
    
    if (currentSubMenu == SubMenu::None) {
        for (size_t i = 0; i < menuItems.size(); i++) {
            if (menuItems[i]) {
                if (static_cast<int>(i) == selectedIndex) {
                    menuItems[i]->setAlpha(1.0f);
                } else {
                    menuItems[i]->setAlpha(0.6f);
                }
                menuItems[i]->update(elapsed);
            }
        }
        
        Controls* controls = GameConfig::getInstance()->controls;
        
        bool upP = controls->justPressedAction(ControlAction::UI_UP);
        bool downP = controls->justPressedAction(ControlAction::UI_DOWN);
        bool accepted = controls->justPressedAction(ControlAction::ACCEPT);
        bool backP = controls->justPressedAction(ControlAction::BACK);
        
        if (upP) changeSelection(-1);
        if (downP) changeSelection(1);
        if (accepted) selectItem();
        if (backP) goBack();
    } else if (currentSubMenu == SubMenu::Preferences) {
        for (size_t i = 0; i < prefLabels.size(); i++) {
            if (prefLabels[i]) {
                prefLabels[i]->update(elapsed);
                if (static_cast<int>(i) == prefSelectedIndex) {
                    prefLabels[i]->setAlpha(1.0f);
                } else {
                    prefLabels[i]->setAlpha(0.6f);
                }
            }
        }
        
        for (auto* checkbox : checkBoxes) {
            if (checkbox) checkbox->update(elapsed);
        }
        
        Controls* controls = GameConfig::getInstance()->controls;
        
        bool upP = controls->justPressedAction(ControlAction::UI_UP);
        bool downP = controls->justPressedAction(ControlAction::UI_DOWN);
        bool accepted = controls->justPressedAction(ControlAction::ACCEPT);
        bool backP = controls->justPressedAction(ControlAction::BACK);
        
        if (upP) {
            flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            prefSelectedIndex--;
            if (prefSelectedIndex < 0) prefSelectedIndex = static_cast<int>(prefLabels.size()) - 1;
        }
        if (downP) {
            flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
            prefSelectedIndex++;
            if (prefSelectedIndex >= static_cast<int>(prefLabels.size())) prefSelectedIndex = 0;
        }
        if (accepted) {
            if (prefSelectedIndex < static_cast<int>(checkBoxes.size())) {
                togglePreference();
            } else {
                exitPreferencesMenu();
            }
        }
        if (backP) {
            exitPreferencesMenu();
        }
    } else if (currentSubMenu == SubMenu::Controls) {
        if (controlsCamera) {
            float lerpSpeed = 0.06f;
            float targetY = camFollowY - (flixel::FlxG::height / 2);
            controlsCamera->scroll.y += (targetY - controlsCamera->scroll.y) * lerpSpeed;
            
            if (controlsCamera->scroll.y < 0) {
                controlsCamera->scroll.y = 0;
            }
        }
        
        if (deviceLabel) deviceLabel->update(elapsed);
        
        for (auto* header : controlHeaders) {
            if (header) header->update(elapsed);
        }
        
        for (size_t i = 0; i < controlLabels.size(); i++) {
            if (controlLabels[i]) {
                controlLabels[i]->update(elapsed);
                if (static_cast<int>(i) == controlSelectedIndex) {
                    controlLabels[i]->setAlpha(1.0f);
                } else {
                    controlLabels[i]->setAlpha(0.6f);
                }
            }
            if (i < controlBindLabels.size() && controlBindLabels[i]) {
                controlBindLabels[i]->update(elapsed);
                if (static_cast<int>(i) == controlSelectedIndex && controlColumnIndex == 0) {
                    controlBindLabels[i]->setAlpha(1.0f);
                } else {
                    controlBindLabels[i]->setAlpha(0.6f);
                }
            }
            if (i < controlBindLabelsAlt.size() && controlBindLabelsAlt[i]) {
                controlBindLabelsAlt[i]->update(elapsed);
                if (static_cast<int>(i) == controlSelectedIndex && controlColumnIndex == 1) {
                    controlBindLabelsAlt[i]->setAlpha(1.0f);
                } else {
                    controlBindLabelsAlt[i]->setAlpha(0.6f);
                }
            }
        }
        
        if (awaitingInput) {
            captureInput();
        } else {
            Controls* controls = GameConfig::getInstance()->controls;
            
            bool upP = controls->justPressedAction(ControlAction::UI_UP);
            bool downP = controls->justPressedAction(ControlAction::UI_DOWN);
            bool leftP = controls->justPressedAction(ControlAction::UI_LEFT);
            bool rightP = controls->justPressedAction(ControlAction::UI_RIGHT);
            bool accepted = controls->justPressedAction(ControlAction::ACCEPT);
            bool backP = controls->justPressedAction(ControlAction::BACK);
            
#ifndef __SWITCH__
            bool qP = flixel::FlxG::keys.keys[SDL_SCANCODE_Q].justPressed();
            bool eP = flixel::FlxG::keys.keys[SDL_SCANCODE_E].justPressed();
            
            if (qP || eP) {
                flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
                isGamepadMode = !isGamepadMode;
                if (deviceLabel) {
                    deviceLabel->setText(isGamepadMode ? "GAMEPAD" : "KEYBOARD");
                    deviceLabel->screenCenter();
                }
                updateControlBindings();
            }
#endif
            if (leftP || rightP) {
                if (controlSelectedIndex < 12) {
                    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
                    controlColumnIndex = (controlColumnIndex == 0) ? 1 : 0;
                }
            }
            if (upP) {
                flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
                controlSelectedIndex--;
                if (controlSelectedIndex < 0) {
                    controlSelectedIndex = static_cast<int>(controlLabels.size()) - 1;
                }
                if (controlSelectedIndex >= 0 && controlSelectedIndex < static_cast<int>(controlLabels.size())) {
                    camFollowY = static_cast<float>(controlLabels[controlSelectedIndex]->getY());
                }
            }
            if (downP) {
                flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
                controlSelectedIndex++;
                if (controlSelectedIndex >= static_cast<int>(controlLabels.size())) {
                    controlSelectedIndex = 0;
                }
                if (controlSelectedIndex >= 0 && controlSelectedIndex < static_cast<int>(controlLabels.size())) {
                    camFollowY = static_cast<float>(controlLabels[controlSelectedIndex]->getY());
                }
            }
            if (accepted) {
                if (controlSelectedIndex < 12) {
                    startRebinding();
                } else {
                    exitControlsMenu();
                }
            }
            if (backP) {
                exitControlsMenu();
            }
        }
    }
}

void OptionsState::draw() {
    if (bg) bg->draw();
    
    if (currentSubMenu == SubMenu::None) {
        drawMainMenu();
    } else if (currentSubMenu == SubMenu::Preferences) {
        drawPreferencesMenu();
    } else if (currentSubMenu == SubMenu::Controls) {
        drawControlsMenu();
    }
    
    FunkinState::draw();
}

void OptionsState::destroy() {
    for (auto* item : menuItems) {
        if (item) delete item;
    }
    menuItems.clear();
    
    for (auto* label : prefLabels) {
        if (label) delete label;
    }
    prefLabels.clear();
    
    for (auto* checkbox : checkBoxes) {
        if (checkbox) delete checkbox;
    }
    checkBoxes.clear();
    
    for (auto* header : controlHeaders) {
        if (header) delete header;
    }
    controlHeaders.clear();
    
    for (auto* label : controlLabels) {
        if (label) delete label;
    }
    controlLabels.clear();
    
    for (auto* label : controlBindLabels) {
        if (label) delete label;
    }
    controlBindLabels.clear();
    
    for (auto* label : controlBindLabelsAlt) {
        if (label) delete label;
    }
    controlBindLabelsAlt.clear();
    
    if (deviceLabel) {
        delete deviceLabel;
        deviceLabel = nullptr;
    }
    
    if (controlsCamera) {
        delete controlsCamera;
        controlsCamera = nullptr;
    }
    
    if (bg) {
        delete bg;
        bg = nullptr;
    }
    
    FunkinState::destroy();
}

void OptionsState::changeSelection(int change) {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
    selectedIndex += change;
    
    if (selectedIndex < 0) selectedIndex = static_cast<int>(menuItems.size()) - 1;
    if (selectedIndex >= static_cast<int>(menuItems.size())) selectedIndex = 0;
}

void OptionsState::selectItem() {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
    
    if (selectedIndex == 0) {
        enterPreferencesMenu();
    } else if (selectedIndex == 1) {
        enterControlsMenu();
    } else if (selectedIndex == 2) {
        // enterModsMenu();
    } else if (selectedIndex == 3) {
        goBack();
    }
}

void OptionsState::goBack() {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
    
    startTransitionOut(0.5f, flixel::util::FlxColor::BLACK, flixel::FlxPoint(0, 1), []() {
        flixel::FlxG::game->switchState(new MainMenuState());
    });
}

void OptionsState::enterPreferencesMenu() {
    currentSubMenu = SubMenu::Preferences;
    prefSelectedIndex = 0;
}

void OptionsState::exitPreferencesMenu() {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
    currentSubMenu = SubMenu::None;
    savePreferences();
}

void OptionsState::enterControlsMenu() {
    currentSubMenu = SubMenu::Controls;
    controlSelectedIndex = 0;
    updateControlBindings();
}

void OptionsState::exitControlsMenu() {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/cancelMenu.ogg");
    currentSubMenu = SubMenu::None;
    
    auto playState = GameConfig::getInstance();
    if (playState && playState->controls) {
        playState->controls->saveToConfig(ASSETS_PATH "assets/data/config.json");
    }
}

void OptionsState::startRebinding() {
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
    awaitingInput = true;
    
    if (controlSelectedIndex >= 0 && controlSelectedIndex < static_cast<int>(controlBindLabels.size())) {
        std::string promptText = "?";
        
        if (controlColumnIndex == 0) {
            controlBindLabels[controlSelectedIndex]->setText(promptText);
        } else if (controlSelectedIndex < static_cast<int>(controlBindLabelsAlt.size())) {
            controlBindLabelsAlt[controlSelectedIndex]->setText(promptText);
        }
    }
}

void OptionsState::captureInput() {
    if (isGamepadMode) {
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
            SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(i);
            if (flixel::FlxG::gamepads.justPressed(button)) {
                if (button == SDL_CONTROLLER_BUTTON_BACK || button == SDL_CONTROLLER_BUTTON_B) {
                    awaitingInput = false;
                    updateControlBindings();
                    return;
                }
                
                auto playState = GameConfig::getInstance();
                if (playState && playState->controls && controlSelectedIndex >= 0 && controlSelectedIndex < 12) {
                    ControlAction action = static_cast<ControlAction>(controlSelectedIndex);
                    bool isPrimary = (controlColumnIndex == 0);
                    playState->controls->rebindGamepad(action, isPrimary, button);
                    awaitingInput = false;
                    updateControlBindings();
                    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
                }
                return;
            }
        }
    } else {
        for (int i = SDL_SCANCODE_A; i < SDL_NUM_SCANCODES; i++) {
            SDL_Scancode scancode = static_cast<SDL_Scancode>(i);
            if (flixel::FlxG::keys.keys[scancode].justPressed()) {
                if (scancode == SDL_SCANCODE_ESCAPE) {
                    awaitingInput = false;
                    updateControlBindings();
                    return;
                }
                
                auto playState = GameConfig::getInstance();
                if (playState && playState->controls && controlSelectedIndex >= 0 && controlSelectedIndex < 12) {
                    ControlAction action = static_cast<ControlAction>(controlSelectedIndex);
                    bool isPrimary = (controlColumnIndex == 0);
                    playState->controls->rebindKey(action, isPrimary, scancode);
                    awaitingInput = false;
                    updateControlBindings();
                    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/confirmMenu.ogg");
                }
                return;
            }
        }
    }
}

void OptionsState::togglePreference() {
    if (prefSelectedIndex < 0 || prefSelectedIndex >= static_cast<int>(checkBoxes.size())) return;
    
    flixel::FlxG::sound.playAsChunk(ASSETS_PATH "assets/sounds/scrollMenu.ogg");
    
    std::vector<std::string> prefKeys = {
        "downscroll", "ghostTapping"
    };
    
    if (prefSelectedIndex < static_cast<int>(prefKeys.size())) {
        std::string key = prefKeys[prefSelectedIndex];
        preferences[key] = !preferences[key];
        checkBoxes[prefSelectedIndex]->setChecked(preferences[key]);
    }
}

void OptionsState::loadPreferences() {
    preferences["downscroll"] = false;
    preferences["ghostTapping"] = true;
    
    std::ifstream file(ASSETS_PATH "assets/data/config.json");
    if (file.is_open()) {
        try {
            json config;
            file >> config;
            file.close();
            
            if (config.contains("gameConfig")) {
                auto& gameConfig = config["gameConfig"];
                
                if (gameConfig.contains("downscroll")) {
                    preferences["downscroll"] = gameConfig["downscroll"];
                }
                if (gameConfig.contains("ghostTapping")) {
                    preferences["ghostTapping"] = gameConfig["ghostTapping"];
                }
                if (gameConfig.contains("cameraZoom")) {
                    preferences["cameraZoom"] = gameConfig["cameraZoom"];
                }
                if (gameConfig.contains("middlescroll")) {
                    preferences["middlescroll"] = gameConfig["middlescroll"];
                }
                if (gameConfig.contains("flashingLights")) {
                    preferences["flashingLights"] = gameConfig["flashingLights"];
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading preferences: " << e.what() << std::endl;
        }
    }
}

void OptionsState::savePreferences() {
    json config;
    std::ifstream inFile(ASSETS_PATH "assets/data/config.json");
    if (inFile.is_open()) {
        try {
            inFile >> config;
        } catch (const std::exception& e) {
            std::cerr << "Error reading config: " << e.what() << std::endl;
            config = json::object();
        }
        inFile.close();
    }
    
    if (!config.contains("gameConfig")) {
        config["gameConfig"] = json::object();
    }
    
    config["gameConfig"]["downscroll"] = preferences["downscroll"];
    config["gameConfig"]["ghostTapping"] = preferences["ghostTapping"];
    
    std::ofstream outFile(ASSETS_PATH "assets/data/config.json");
    if (outFile.is_open()) {
        outFile << config.dump(4);
        outFile.close();
        std::cout << "Preferences saved!" << std::endl;
    } else {
        std::cerr << "Failed to save preferences!" << std::endl;
    }
}

void OptionsState::drawMainMenu() {
    for (auto* item : menuItems) {
        if (item) item->draw();
    }
}

void OptionsState::drawPreferencesMenu() {
    for (size_t i = 0; i < checkBoxes.size(); i++) {
        if (checkBoxes[i]) {
            checkBoxes[i]->draw();
        }
    }
    
    for (auto* label : prefLabels) {
        if (label) label->draw();
    }
}

void OptionsState::drawControlsMenu() {
    if (deviceLabel) deviceLabel->draw();
    
    for (auto* header : controlHeaders) {
        if (header) header->draw();
    }
    
    for (size_t i = 0; i < controlLabels.size(); i++) {
        if (controlLabels[i]) controlLabels[i]->draw();
        if (i < controlBindLabels.size() && controlBindLabels[i]) {
            controlBindLabels[i]->draw();
        }
        if (i < controlBindLabelsAlt.size() && controlBindLabelsAlt[i]) {
            controlBindLabelsAlt[i]->draw();
        }
    }
}

void OptionsState::updateControlBindings() {
    auto playState = GameConfig::getInstance();
    if (!playState || !playState->controls) return;
    
    std::vector<ControlAction> actions = {
        ControlAction::NOTE_LEFT,
        ControlAction::NOTE_DOWN,
        ControlAction::NOTE_UP,
        ControlAction::NOTE_RIGHT,
        ControlAction::UI_LEFT,
        ControlAction::UI_DOWN,
        ControlAction::UI_UP,
        ControlAction::UI_RIGHT,
        ControlAction::ACCEPT,
        ControlAction::BACK,
        ControlAction::PAUSE,
        ControlAction::RESET
    };
    
    for (size_t i = 0; i < actions.size() && i < controlBindLabels.size(); i++) {
        std::string primaryText;
        std::string alternateText;
        
        if (isGamepadMode) {
            auto padBind = playState->controls->getGamepadBindForAction(actions[i]);
            
            if (padBind.primary != SDL_CONTROLLER_BUTTON_INVALID) {
                primaryText = Controls::getStringFromButton(padBind.primary);
            } else {
                primaryText = "---";
            }
            
            if (padBind.alternate != SDL_CONTROLLER_BUTTON_INVALID) {
                alternateText = Controls::getStringFromButton(padBind.alternate);
            } else {
                alternateText = "---";
            }
        } else {
            auto keyBind = playState->controls->getKeyBindForAction(actions[i]);
            
            if (keyBind.primary != SDL_SCANCODE_UNKNOWN) {
                primaryText = Controls::getStringFromScancode(keyBind.primary);
            } else {
                primaryText = "---";
            }
            
            if (keyBind.alternate != SDL_SCANCODE_UNKNOWN) {
                alternateText = Controls::getStringFromScancode(keyBind.alternate);
            } else {
                alternateText = "---";
            }
        }
        
        controlBindLabels[i]->setText(primaryText);
        if (i < controlBindLabelsAlt.size()) {
            controlBindLabelsAlt[i]->setText(alternateText);
        }
    }
}