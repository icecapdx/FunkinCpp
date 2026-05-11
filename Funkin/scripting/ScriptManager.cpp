#include "ScriptManager.h"
#include "ScriptAPI.h"
#include <filesystem>
#include <iostream>
#include <flixel/FlxSprite.h>
#include <flixel/text/FlxText.h>

ScriptManager* ScriptManager::instance = nullptr;

ScriptManager::ScriptManager() {
    ScriptAPI::initialize();
}

ScriptManager::~ScriptManager() {
    clear();
}

ScriptManager* ScriptManager::getInstance() {
    if (!instance) {
        instance = new ScriptManager();
    }
    return instance;
}

void ScriptManager::loadScript(const std::string& scriptPath) {
    auto script = std::make_unique<Script>(scriptPath);
    if (script->load()) {
        std::cout << "Loaded script: " << scriptPath << std::endl;
        scripts.push_back(std::move(script));
    } else {
        std::cerr << "Failed to load script: " << scriptPath << std::endl;
    }
}

void ScriptManager::loadScriptsFromDirectory(const std::string& directory) {
    if (!std::filesystem::exists(directory)) {
        std::cerr << "Script directory does not exist: " << directory << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string pathStr = entry.path().string();
            
            if (pathStr.find("\\songs\\") != std::string::npos || pathStr.find("/songs/") != std::string::npos ||
                pathStr.find("\\characters\\") != std::string::npos || pathStr.find("/characters/") != std::string::npos ||
                pathStr.find("\\stages\\") != std::string::npos || pathStr.find("/stages/") != std::string::npos) {
                continue;
            }
            
            loadScript(pathStr);
        }
    }
}

void ScriptManager::loadSongScripts(const std::string& songName) {
    std::string scriptPath = ASSETS_PATH "assets/scripts/songs/" + songName + ".txt";
    if (std::filesystem::exists(scriptPath)) {
        loadScript(scriptPath);
    }
}

void ScriptManager::loadCharacterScripts(const std::vector<std::string>& characterNames) {
    std::vector<std::string> loadedPaths;
    
    for (const auto& charName : characterNames) {
        std::string scriptPath = ASSETS_PATH "assets/scripts/characters/" + charName + ".txt";
        
        std::string normalizedPath = scriptPath;
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        
        bool alreadyLoaded = false;
        for (const auto& loaded : loadedPaths) {
            std::string normalizedLoaded = loaded;
            std::replace(normalizedLoaded.begin(), normalizedLoaded.end(), '\\', '/');
            if (normalizedLoaded == normalizedPath) {
                alreadyLoaded = true;
                break;
            }
        }
        
        if (alreadyLoaded) {
            continue;
        }
        
        if (std::filesystem::exists(scriptPath)) {
            loadScript(scriptPath);
            loadedPaths.push_back(scriptPath);
        }
    }
}

void ScriptManager::loadStageScripts(const std::string& stageName) {
    std::string scriptPath = ASSETS_PATH "assets/scripts/stages/" + stageName + ".txt";
    if (std::filesystem::exists(scriptPath)) {
        loadScript(scriptPath);
    }
}

void ScriptManager::clear() {
    scripts.clear();
    ScriptAPI::cleanup();
}

void ScriptManager::callAll(ScriptCallback callback, const std::vector<ScriptValue>& args) {
    for (auto& script : scripts) {
        if (script && script->isLoaded()) {
            script->call(callback, args);
        }
    }
}

void ScriptManager::setGlobalVar(const std::string& name, const ScriptValue& value) {
    globalVars.set(name, value);
}

ScriptValue ScriptManager::getGlobalVar(const std::string& name, const ScriptValue& defaultValue) const {
    return globalVars.get(name, defaultValue);
}

void ScriptManager::drawScriptObjects() {
    for (auto& pair : ScriptAPI::getSprites()) {
        if (pair.second && pair.second->visible) {
            pair.second->draw();
        }
    }

    for (auto& pair : ScriptAPI::getTexts()) {
        if (pair.second && pair.second->visible) {
            pair.second->draw();
        }
    }
}

void ScriptManager::updateScriptObjects(float elapsed) {
    for (auto& pair : ScriptAPI::getSprites()) {
        if (pair.second) {
            pair.second->update(elapsed);
        }
    }

    for (auto& pair : ScriptAPI::getTexts()) {
        if (pair.second) {
            pair.second->update(elapsed);
        }
    }
}
