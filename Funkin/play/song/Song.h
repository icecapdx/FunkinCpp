#pragma once
#include <string>
#include <vector>
#include "../../game/Section.h"

struct SwagSong {
    std::string song;
    std::vector<SwagSection> notes;
    int bpm;
    bool needsVoices = true;
    float speed = 1.0f;
    std::string player1 = "bf";
    std::string player2 = "dad";
    std::string gfVersion = "gf";
    std::string stage = "";
    bool validScore = false;
    bool isVSlice = false;
};

class Song {
public:
    std::string song;
    std::vector<SwagSection> notes;
    int bpm;
    bool needsVoices = true;
    float speed = 1.0f;
    std::string player1 = "bf";
    std::string player2 = "dad";
    std::string gfVersion = "gf";
    Song(const std::string& song, const std::vector<SwagSection>& notes, int bpm);

    static SwagSong loadFromJson(const std::string& jsonInput, const std::string& folder = "");
    static SwagSong parseJSONshit(const std::string& rawJson);
};
