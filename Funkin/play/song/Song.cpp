#include "Song.h"
#include <Tsukiyo/ChartLoader.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

Song::Song(const std::string& song, const std::vector<SwagSection>& notes, int bpm)
    : song(song), notes(notes), bpm(bpm) {
}

static SwagSong convertTsukiyoToSwagSong(const Tsukiyo::Chart& chart, const std::string& songName) {
    SwagSong swagShit;
    
    swagShit.song = chart.title.empty() ? songName : chart.title;
    swagShit.bpm = static_cast<int>(chart.bpm);
    swagShit.speed = chart.speed;
    swagShit.needsVoices = true;
    
    for (const auto& section : chart.sections) {
        SwagSection swagSection;
        swagSection.lengthInSteps = section.lengthInSteps;
        swagSection.mustHitSection = section.mustHitSection;
        swagSection.bpm = static_cast<int>(section.bpm);
        swagSection.changeBPM = section.changeBPM;
        
        for (const auto& note : section.notes) {
            std::vector<float> noteData;
            noteData.push_back(note.time);
            noteData.push_back(static_cast<float>(note.lane));
            noteData.push_back(note.duration);
            swagSection.sectionNotes.push_back(noteData);
        }
        
        swagShit.notes.push_back(swagSection);
    }
    
    swagShit.validScore = true;
    return swagShit;
}

static bool tryLoadVSlice(const std::string& basePath, const std::string& songName, const std::string& difficulty, SwagSong& outSong) {
    std::string chartPath = basePath + "/" + songName + "-chart.json";
    std::string metaPath = basePath + "/" + songName + "-metadata.json";
    
    if (!std::filesystem::exists(chartPath) || !std::filesystem::exists(metaPath)) {
        return false;
    }
    
    std::cerr << "Loading V-Slice chart: " << chartPath << std::endl;
    
    auto chart = Tsukiyo::Chart::createChart(Tsukiyo::Chart::Format::FNFVSlice);
    if (!chart || !chart->loadFromFile(chartPath)) {
        return false;
    }
    
    auto* vslice = dynamic_cast<Tsukiyo::FNFVSliceChart*>(chart.get());
    if (vslice) {
        auto difficulties = vslice->getAvailableDifficulties();
        std::cerr << "Available difficulties: ";
        for (const auto& d : difficulties) {
            std::cerr << d << " ";
        }
        std::cerr << std::endl;
        
        std::cerr << "Selecting difficulty: " << difficulty << std::endl;
        if (!vslice->selectDifficulty(difficulty)) {
            std::cerr << "Failed to select difficulty: " << difficulty << ", using default" << std::endl;
        }
    }
    
    outSong = convertTsukiyoToSwagSong(*chart, songName);
    outSong.isVSlice = true;
    
    try {
        std::ifstream metaFile(metaPath);
        nlohmann::json metaJson = nlohmann::json::parse(metaFile);
        
        if (metaJson.contains("playData") && metaJson["playData"].is_object()) {
            auto playData = metaJson["playData"];
            if (playData.contains("characters") && playData["characters"].is_object()) {
                auto chars = playData["characters"];
                outSong.player1 = chars.value("player", "bf");
                outSong.player2 = chars.value("opponent", "dad");
                outSong.gfVersion = chars.value("girlfriend", "gf");
            }
            outSong.stage = playData.value("stage", "mainStage");
        }
    } catch (...) {
    }
    
    return true;
}

static bool tryLoadLegacy(const std::string& path, const std::string& songName, SwagSong& outSong) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    std::cerr << "Loading Legacy chart: " << path << std::endl;
    
    auto chart = Tsukiyo::Chart::createChart(Tsukiyo::Chart::Format::FNFLegacy);
    if (!chart || !chart->loadFromFile(path)) {
        return false;
    }
    
    outSong = convertTsukiyoToSwagSong(*chart, songName);
    
    try {
        std::ifstream file(path);
        std::string rawJson((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        while (!rawJson.empty() && std::isspace(rawJson.back())) {
            rawJson.pop_back();
        }
        while (!rawJson.empty() && rawJson.back() != '}') {
            rawJson.pop_back();
        }
        
        nlohmann::json j = nlohmann::json::parse(rawJson);
        nlohmann::json songData = j.contains("song") ? j["song"] : j;
        
        outSong.player1 = songData.value("player1", "bf");
        if (outSong.player1.empty()) outSong.player1 = "bf";
        
        outSong.player2 = songData.value("player2", "dad");
        if (outSong.player2.empty()) outSong.player2 = "dad";
        
        outSong.gfVersion = songData.value("gfVersion", "");
        if (outSong.gfVersion.empty()) {
            outSong.gfVersion = songData.value("player3", "gf");
        }
        if (outSong.gfVersion.empty()) outSong.gfVersion = "gf";
        
        outSong.stage = songData.value("stage", "");
        outSong.needsVoices = songData.value("needsVoices", true);
    } catch (...) {
    }
    
    return true;
}

SwagSong Song::loadFromJson(const std::string& songName, const std::string& folder) {
    SwagSong song;
    std::string difficulty = "normal";
    std::string lowerSongName = songName;
    std::transform(lowerSongName.begin(), lowerSongName.end(), lowerSongName.begin(), ::tolower);
    
    bool isEasy = (lowerSongName.length() >= 5 && lowerSongName.substr(lowerSongName.length() - 5) == "-easy");
    bool isHard = (lowerSongName.length() >= 5 && lowerSongName.substr(lowerSongName.length() - 5) == "-hard");
    
    if (isEasy) {
        difficulty = "easy";
    } else if (isHard) {
        difficulty = "hard";
    }
    
    std::string lowerFolder = folder;
    std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);
    
    std::string baseSongName = lowerFolder.empty() ? lowerSongName : lowerFolder;
    size_t dashPos = baseSongName.rfind("-");
    if (dashPos != std::string::npos) {
        std::string suffix = baseSongName.substr(dashPos);
        if (suffix == "-easy" || suffix == "-hard") {
            baseSongName = baseSongName.substr(0, dashPos);
        }
    }
    
    std::cerr << "Loading song: " << baseSongName << " difficulty: " << difficulty << std::endl;
    
    std::string vslicePath = "assets/data/songs/" + baseSongName;
    if (tryLoadVSlice(vslicePath, baseSongName, difficulty, song)) {
        std::cerr << "Loaded V-Slice chart successfully" << std::endl;
        return song;
    }
    
    std::string legacyPath = "assets/songs/" + baseSongName + "/" + lowerSongName + ".json";
    std::cerr << "Trying V-Slice path: " << vslicePath << std::endl;
    std::cerr << "Trying Legacy path: " << legacyPath << std::endl;
    
    if (tryLoadLegacy(legacyPath, baseSongName, song)) {
        std::cerr << "Loaded Legacy chart successfully" << std::endl;
        return song;
    }
    
    std::cerr << "Failed to load chart: " << songName << " (neither V-Slice nor Legacy found)" << std::endl;
    return SwagSong();
}

SwagSong Song::parseJSONshit(const std::string& rawJson) {
    SwagSong swagShit;
    try {
        std::string cleanJson = rawJson;
        while (!cleanJson.empty() && std::isspace(cleanJson.back())) {
            cleanJson.pop_back();
        }
        while (!cleanJson.empty() && cleanJson.back() != '}') {
            cleanJson.pop_back();
        }
        
        nlohmann::json j = nlohmann::json::parse(cleanJson);
        nlohmann::json songData = j.contains("song") ? j["song"] : j;

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
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "JSON parsing error: " << ex.what() << std::endl;
        return SwagSong();
    }

    return swagShit;
}