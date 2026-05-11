#include "SongLoader.h"
#include "Conductor.h"
#include <iostream>

SwagSong SongLoader::loadSong(const std::string& dataPath) {
    std::string songName, folder, baseSongName;
    parseSongPath(dataPath, songName, folder, baseSongName);
    
    SwagSong song = Song::loadFromJson(songName, folder);
    if (!song.validScore) {
        std::cerr << "Failed to load song data" << std::endl;
    } else {
        Conductor::changeBPM(static_cast<float>(song.bpm));
    }
    
    return song;
}

void SongLoader::loadSongAudio(const std::string& songName, flixel::FlxSound*& inst, flixel::FlxSound*& vocals, const SwagSong& song) {
    std::string baseSongName = songName;
    
    if (baseSongName.length() >= 5 && (baseSongName.substr(baseSongName.length() - 5) == "-easy" ||
                                        baseSongName.substr(baseSongName.length() - 5) == "-hard")) {
        size_t dashPos = baseSongName.rfind("-");
        if (dashPos != std::string::npos) {
            baseSongName = baseSongName.substr(0, dashPos);
        }
    }
    
    std::string soundExt = ".ogg";
    std::string vocalsPath = song.needsVoices ? ASSETS_PATH "assets/songs/" + baseSongName + "/Voices" + soundExt : "";
    std::string instPath = ASSETS_PATH "assets/songs/" + baseSongName + "/Inst" + soundExt;
    
    if (inst != nullptr) {
        delete inst;
        inst = nullptr;
    }
    inst = new flixel::FlxSound();
    if (!inst->loadAsChunk(instPath, false, false)) {
        std::cerr << "Failed to preload instrumental: " << instPath << std::endl;
        delete inst;
        inst = nullptr;
    }
    if (inst) {
        inst->setChannel(1);
        inst->setVolume(1.0f);
        inst->stop();
    }
    
    if (!vocalsPath.empty()) {
        if (vocals != nullptr) {
            delete vocals;
            vocals = nullptr;
        }
        vocals = new flixel::FlxSound();
        if (!vocals->loadAsChunk(vocalsPath, false, false)) {
            std::cerr << "Failed to preload vocals: " << vocalsPath << std::endl;
            delete vocals;
            vocals = nullptr;
        } else {
            vocals->setChannel(0);
            vocals->setVolume(1.0f);
            vocals->stop();
        }
    }
}

void SongLoader::parseSongPath(const std::string& dataPath, std::string& songName, std::string& folder, std::string& baseSongName) {
    songName = dataPath;
    folder = dataPath;
    baseSongName = dataPath;
    
    if (folder.length() >= 5 && (folder.substr(folder.length() - 5) == "-easy" ||
                                  folder.substr(folder.length() - 5) == "-hard")) {
        size_t dashPos = folder.rfind("-");
        if (dashPos != std::string::npos) {
            folder = folder.substr(0, dashPos);
            baseSongName = folder;
        }
    }
}