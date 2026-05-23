#include "Song.h"
#include <Tsukiyo/ChartLoader.hpp>
#include <Tsukiyo/formats/FNFLegacyChart.hpp>
#include <Tsukiyo/formats/FNFVSliceChart.hpp>
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

static std::vector<SwagSection> convertSections(const std::vector<Tsukiyo::Section>& src) {
    std::vector<SwagSection> out;
    out.reserve(src.size());
    for (const auto& sec : src) {
        SwagSection s;
        s.lengthInSteps  = sec.lengthInSteps;
        s.mustHitSection = sec.mustHitSection;
        s.changeBPM      = sec.changeBPM;
        s.bpm            = static_cast<int>(sec.bpm);
        for (const auto& note : sec.notes) {
            s.sectionNotes.push_back({note.time, static_cast<float>(note.lane), note.duration});
        }
        out.push_back(std::move(s));
    }
    return out;
}

SwagSong Song::loadFromJson(const std::string& songName, const std::string& folder) {
    SwagSong song;

    std::string actualFolder = folder;
    std::string lowerSongName = songName;
    std::transform(lowerSongName.begin(), lowerSongName.end(), lowerSongName.begin(), ::tolower);

    for (const char* suf : {"-easy", "-hard"}) {
        if (actualFolder.size() > 5 &&
            actualFolder.substr(actualFolder.size() - 5) == suf)
        {
            actualFolder = actualFolder.substr(0, actualFolder.rfind('-'));
            break;
        }
    }

    std::string lowerFolder = actualFolder;
    std::transform(lowerFolder.begin(), lowerFolder.end(), lowerFolder.begin(), ::tolower);

    std::string songBase = lowerSongName;
    for (const char* suf : {"-easy", "-hard"}) {
        if (songBase.size() > 5 && songBase.substr(songBase.size() - 5) == suf)
            songBase = songBase.substr(0, songBase.rfind('-'));
    }

    const std::string base = ASSETS_PATH "assets/data/songs/" + lowerFolder + "/";

    const std::string vsliceChartPath = base + songBase + "-chart.json";
    const std::string vsliceMetaPath  = base + songBase + "-metadata.json";
    const bool isVSlice = std::filesystem::exists(vsliceChartPath) &&
                          std::filesystem::exists(vsliceMetaPath);

    if (isVSlice) {
        std::cout << "[Song] Loading V-Slice chart: " << vsliceChartPath << std::endl;

        Tsukiyo::FNFVSliceChart chart;
        if (!chart.loadFromFile(vsliceChartPath)) {
            std::cerr << "[Song] Failed to load V-Slice chart: " << vsliceChartPath << std::endl;
            return song;
        }

        std::string diff = "normal";
        if (lowerSongName.size() > 5) {
            if (lowerSongName.substr(lowerSongName.size() - 5) == "-easy") diff = "easy";
            else if (lowerSongName.substr(lowerSongName.size() - 5) == "-hard") diff = "hard";
        }
        chart.selectDifficulty(diff);

        song.song       = chart.title;
        song.bpm        = static_cast<int>(chart.bpm);
        song.speed      = chart.speed;
        song.notes      = convertSections(chart.sections);
        song.isVSlice   = true;

        std::ifstream metaFile(vsliceMetaPath);
        if (metaFile.is_open()) {
            try {
                json meta = json::parse(metaFile);
                if (meta.contains("playData") && meta["playData"].is_object()) {
                    const auto& pd = meta["playData"];

                    if (pd.contains("characters") && pd["characters"].is_object()) {
                        const auto& ch = pd["characters"];
                        song.player1   = ch.value("player", "bf");
                        song.player2   = ch.value("opponent", "dad");
                        song.gfVersion = ch.value("girlfriend", "gf");
                    }
                    song.stage = pd.value("stage", "");
                }
                song.needsVoices = true;
            } catch (const json::exception& ex) {
                std::cerr << "[Song] Failed to parse V-Slice metadata: " << ex.what() << std::endl;
            }
        }

        if (song.player1.empty())   song.player1   = "bf";
        if (song.player2.empty())   song.player2   = "dad";
        if (song.gfVersion.empty()) song.gfVersion = "gf";

    } else {
        const std::string legacyPath = base + lowerSongName + ".json";
        std::cout << "[Song] Loading Legacy chart: " << legacyPath << std::endl;

        Tsukiyo::FNFLegacyChart chart;
        if (!chart.loadFromFile(legacyPath)) {
            std::cerr << "[Song] Failed to load Legacy chart: " << legacyPath << std::endl;
            return song;
        }

        song.song   = chart.title;
        song.bpm    = static_cast<int>(chart.bpm);
        song.speed  = chart.speed;
        song.notes  = convertSections(chart.sections);

        std::ifstream file(legacyPath);
        if (file.is_open()) {
            try {
                std::string raw((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
                while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.back())))
                    raw.pop_back();
                while (!raw.empty() && raw.back() != '}')
                    raw.pop_back();

                json j = json::parse(raw);
                json& sd = j.contains("song") && j["song"].is_object() ? j["song"] : j;

                song.needsVoices = sd.value("needsVoices", true);
                song.player1     = sd.value("player1", "bf");
                song.player2     = sd.value("player2", "dad");
                song.gfVersion   = sd.value("gfVersion", "");
                if (song.gfVersion.empty())
                    song.gfVersion = sd.value("player3", "gf");
                song.stage = sd.value("stage", "");
            } catch (const json::exception& ex) {
                std::cerr << "[Song] Failed to re-read legacy metadata: " << ex.what() << std::endl;
            }
        }

        if (song.player1.empty())   song.player1   = "bf";
        if (song.player2.empty())   song.player2   = "dad";
        if (song.gfVersion.empty()) song.gfVersion = "gf";
    }

    song.validScore = true;
    return song;
}

SwagSong Song::parseJSONshit(const std::string& rawJson) {
    SwagSong swagShit;
    try {
        json j = json::parse(rawJson);
        json songData = j["song"];

        swagShit.song = songData.value("song", "");
        swagShit.bpm  = static_cast<int>(songData.value("bpm", 100.0f));
        swagShit.needsVoices = songData.value("needsVoices", true);
        swagShit.speed       = songData.value("speed", 1.0f);
        swagShit.player1     = songData.value("player1", "bf");
        if (swagShit.player1.empty()) swagShit.player1 = "bf";
        swagShit.player2 = songData.value("player2", "dad");
        if (swagShit.player2.empty()) swagShit.player2 = "dad";
        swagShit.gfVersion = songData.value("gfVersion", "");
        if (swagShit.gfVersion.empty())
            swagShit.gfVersion = songData.value("player3", "gf");
        if (swagShit.gfVersion.empty()) swagShit.gfVersion = "gf";
        swagShit.stage = songData.value("stage", "");

        for (const auto& noteJson : songData.value("notes", json::array())) {
            SwagSection section;
            section.lengthInSteps  = noteJson.value("lengthInSteps", 16);
            section.mustHitSection = noteJson.value("mustHitSection", true);
            section.typeOfSection  = noteJson.value("typeOfSection", 0);
            section.bpm            = noteJson.value("bpm", 0);
            section.changeBPM      = noteJson.value("changeBPM", false);
            section.altAnim        = noteJson.value("altAnim", false);

            for (const auto& na : noteJson.value("sectionNotes", json::array())) {
                if (!na.is_array()) continue;
                std::vector<float> note;
                for (const auto& v : na)
                    if (v.is_number()) note.push_back(v.get<float>());
                if (!note.empty()) section.sectionNotes.push_back(note);
            }
            swagShit.notes.push_back(section);
        }

        swagShit.validScore = true;
    } catch (const json::exception& ex) {
        std::cerr << "JSON parsing error: " << ex.what() << std::endl;
        return SwagSong();
    }
    return swagShit;
}
