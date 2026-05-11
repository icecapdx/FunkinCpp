#pragma once

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "../imports.h"

class Controls;

class GameConfig {
private:
    static GameConfig* instance;
    nlohmann::json config;
    bool downscroll;
    bool ghostTapping;

    GameConfig();
    ~GameConfig();
    void loadConfig();

public:
    Controls* controls;
    
    static GameConfig* getInstance();
    
    bool isDownscroll() const { return downscroll; }
    bool isGhostTapping() const { return ghostTapping; }
    
    void setDownscroll(bool value);
    void setGhostTapping(bool value);
    void saveConfig();
};
