#include "Song.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "../../imports.h"

using json = nlohmann::json;

Song::Song(const std::string& song, const std::vector<SwagSection>& notes, int bpm)
    : song(song), notes(notes), bpm(bpm) {
}

SwagSong Song::loadFromJson(const std::string& songName, const std::string& folder) {
    SwagSong song;
    std::string actualFolder = folder;
    
    bool isEasy = (actualFolder.length() >= 5 && actualFolder.substr(actualFolder.length() - 5) == "-easy");
    bool isHard = (actualFolder.length() >= 5 && actualFolder.substr(actualFolder.length() - 5) == "-hard");
    
    if (isEasy || isHard) {
        size_t dashPos = actualFolder.rfind("-");
        if (dashPos != std::string::npos) {
            actualFolder = actualFolder.substr(0, dashPos);
        }
    }
    
    std::string lowerFolder = actualFolder;
    std::string lowerSongName = songName;
    
    std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
    std::transform(lowerSongName.begin(), lowerSongName.end(), lowerSongName.begin(), ::tolower);
    
    std::string path = ASSETS_PATH "assets/songs/";
    if (!lowerFolder.empty()) {
        path += lowerFolder + "/";
    }
    path += lowerSongName + ".json";

    std::cout << "Final path: " << path << std::endl;

    std::string rawJson;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        return SwagSong();
    }
    
    rawJson = std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();

    while (!rawJson.empty() && std::isspace(rawJson.back())) {
        rawJson.pop_back();
    }

    while (!rawJson.empty() && rawJson.back() != '}') {
        rawJson.pop_back();
    }

    return parseJSONshit(rawJson);
}

SwagSong Song::parseJSONshit(const std::string& rawJson) {
    SwagSong swagShit;
    try {
        json j = json::parse(rawJson);
        json songData = j["song"];

        swagShit.song = songData.value("song", "");
        swagShit.bpm = static_cast<int>(songData.value("bpm", 100.0f));
        swagShit.needsVoices = songData.value("needsVoices", true);
        swagShit.speed = songData.value("speed", 1.0f);
        swagShit.player1 = songData.value("player1", "bf");
        if (swagShit.player1.empty()) swagShit.player1 = "bf";
        swagShit.player2 = songData.value("player2", "dad");
        if (swagShit.player2.empty()) swagShit.player2 = "dad";        
        swagShit.gfVersion = songData.value("gfVersion", "");
        if (swagShit.gfVersion.empty()) {
            swagShit.gfVersion = songData.value("player3", "gf");
        }
        if (swagShit.gfVersion.empty()) swagShit.gfVersion = "gf";
        
        swagShit.stage = songData.value("stage", "");
        
        if (songData.contains("notes") && songData["notes"].is_array()) {
            for (const auto& noteJson : songData["notes"]) {
                SwagSection section;
                section.lengthInSteps = noteJson.value("lengthInSteps", 16);
                section.mustHitSection = noteJson.value("mustHitSection", true);
                section.typeOfSection = noteJson.value("typeOfSection", 0);
                section.bpm = noteJson.value("bpm", 0);
                section.changeBPM = noteJson.value("changeBPM", false);
                section.altAnim = noteJson.value("altAnim", false);
                
                if (noteJson.contains("sectionNotes") && noteJson["sectionNotes"].is_array()) {
                    for (const auto& noteArray : noteJson["sectionNotes"]) {
                        if (!noteArray.is_array()) continue;
                        
                        std::vector<float> note;
                        for (const auto& value : noteArray) {
                            if (value.is_number()) {
                                note.push_back(value.get<float>());
                            }
                        }
                        if (!note.empty()) {
                            section.sectionNotes.push_back(note);
                        }
                    }
                }
                
                swagShit.notes.push_back(section);
            }
        }

        swagShit.validScore = true;
    } catch (const json::exception& ex) {
        std::cerr << "JSON parsing error: " << ex.what() << std::endl;
        return SwagSong();
    }

    return swagShit;
}
