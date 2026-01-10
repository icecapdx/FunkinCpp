#include "SongLoader.h"
#include "Conductor.h"
#include <iostream>
#include <filesystem>

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

flixel::FlxSound* SongLoader::tryLoadSound(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return nullptr;
    }
    
    auto* sound = new flixel::FlxSound();
    if (!sound->loadAsChunk(path, false, false)) {
        delete sound;
        return nullptr;
    }
    sound->setVolume(1.0f);
    sound->stop();
    return sound;
}

void SongLoader::loadSongAudio(const std::string& songName, flixel::FlxSound*& inst, 
                               flixel::FlxSound*& vocalsPlayer, flixel::FlxSound*& vocalsOpponent,
                               const SwagSong& song) {
    std::string baseSongName = songName;
    
    if (baseSongName.length() >= 5 && (baseSongName.substr(baseSongName.length() - 5) == "-easy" ||
                                        baseSongName.substr(baseSongName.length() - 5) == "-hard")) {
        size_t dashPos = baseSongName.rfind("-");
        if (dashPos != std::string::npos) {
            baseSongName = baseSongName.substr(0, dashPos);
        }
    }
    
    std::string soundExt = ".ogg";
    std::string songsPath = "assets/songs/" + baseSongName + "/";
    
    if (inst != nullptr) {
        delete inst;
        inst = nullptr;
    }
    if (vocalsPlayer != nullptr) {
        delete vocalsPlayer;
        vocalsPlayer = nullptr;
    }
    if (vocalsOpponent != nullptr) {
        delete vocalsOpponent;
        vocalsOpponent = nullptr;
    }
    
    std::string instPath = songsPath + "Inst" + soundExt;
    inst = tryLoadSound(instPath);
    if (inst) {
        inst->setChannel(1);
    } else {
        std::cerr << "Failed to preload instrumental: " << instPath << std::endl;
    }
    
    if (!song.needsVoices) {
        return;
    }
    
    if (song.isVSlice) {
        std::string playerChar = song.player1;
        std::string opponentChar = song.player2;
        
        std::string playerVocalsPath = songsPath + "Voices-" + playerChar + soundExt;
        vocalsPlayer = tryLoadSound(playerVocalsPath);
        
        if (!vocalsPlayer && playerChar.length() > 9 && 
            playerChar.substr(playerChar.length() - 9) == "-playable") {
            std::string baseChar = playerChar.substr(0, playerChar.length() - 9);
            playerVocalsPath = songsPath + "Voices-" + baseChar + soundExt;
            vocalsPlayer = tryLoadSound(playerVocalsPath);
        }
        
        if (vocalsPlayer) {
            vocalsPlayer->setChannel(2);
            std::cerr << "Loaded player vocals: " << playerVocalsPath << std::endl;
        }
        
        std::string opponentVocalsPath = songsPath + "Voices-" + opponentChar + soundExt;
        vocalsOpponent = tryLoadSound(opponentVocalsPath);
        
        if (!vocalsOpponent && opponentChar.length() > 9 && 
            opponentChar.substr(opponentChar.length() - 9) == "-playable") {
            std::string baseChar = opponentChar.substr(0, opponentChar.length() - 9);
            opponentVocalsPath = songsPath + "Voices-" + baseChar + soundExt;
            vocalsOpponent = tryLoadSound(opponentVocalsPath);
        }
        
        if (vocalsOpponent) {
            vocalsOpponent->setChannel(3);
            std::cerr << "Loaded opponent vocals: " << opponentVocalsPath << std::endl;
        }
        
        if (!vocalsPlayer && !vocalsOpponent) {
            std::string legacyVocalsPath = songsPath + "Voices" + soundExt;
            vocalsPlayer = tryLoadSound(legacyVocalsPath);
            if (vocalsPlayer) {
                vocalsPlayer->setChannel(2);
                std::cerr << "Fell back to legacy vocals: " << legacyVocalsPath << std::endl;
            }
        }
    } else {
        std::string vocalsPath = songsPath + "Voices" + soundExt;
        vocalsPlayer = tryLoadSound(vocalsPath);
        if (vocalsPlayer) {
            vocalsPlayer->setChannel(2);
            std::cerr << "Loaded vocals: " << vocalsPath << std::endl;
        } else {
            std::cerr << "Failed to preload vocals: " << vocalsPath << std::endl;
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