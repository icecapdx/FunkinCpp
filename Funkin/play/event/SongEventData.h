#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct SongEvent {
    float time = 0.0f;
    std::string kind;
    nlohmann::json value;
    bool activated = false;
};
