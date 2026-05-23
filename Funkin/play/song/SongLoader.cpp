#include "SongLoader.h"
#include "Conductor.h"
#include "../../imports.h"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cstring>

SwagSong SongLoader::loadSong(const std::string& dataPath) {
    std::string songName, folder, baseSongName;
    parseSongPath(dataPath, songName, folder, baseSongName);

    SwagSong song = Song::loadFromJson(songName, folder);
    if (!song.validScore) {
        std::cerr << "[SongLoader] Failed to load song data for: " << dataPath << std::endl;
    } else {
        Conductor::changeBPM(static_cast<float>(song.bpm));
    }
    return song;
}

flixel::FlxSound* SongLoader::tryLoadVoice(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::cout << "[SongLoader] Voice not found, skipping: " << path << std::endl;
        return nullptr;
    }

    auto* snd = new flixel::FlxSound();
    if (!snd->loadAsChunk(path, false, false)) {
        std::cerr << "[SongLoader] Failed to load voice chunk: " << path << std::endl;
        delete snd;
        return nullptr;
    }
    std::cout << "[SongLoader] Loaded voice: " << path << std::endl;
    snd->setVolume(1.0f);
    snd->stop();
    return snd;
}

void SongLoader::loadSongAudio(const std::string& songName,
                               flixel::FlxSound*& inst,
                               flixel::FlxSound*& playerVocals,
                               flixel::FlxSound*& opponentVocals,
                               const SwagSong& song) {
    std::string baseSongName = songName;
    for (const char* suf : {"-easy", "-hard"}) {
        size_t sufLen = strlen(suf);
        if (baseSongName.size() >= sufLen &&
            baseSongName.substr(baseSongName.size() - sufLen) == suf)
        {
            baseSongName = baseSongName.substr(0, baseSongName.rfind('-'));
            break;
        }
    }

    const std::string soundExt = ".ogg";
    const std::string base = ASSETS_PATH "assets/songs/" + baseSongName + "/";

    if (inst != nullptr) { delete inst; inst = nullptr; }

    inst = new flixel::FlxSound();
    const std::string instPath = base + "Inst" + soundExt;
    if (!inst->loadAsChunk(instPath, false, false)) {
        std::cerr << "[SongLoader] Failed to load inst: " << instPath << std::endl;
        delete inst;
        inst = nullptr;
    } else {
        inst->setChannel(1);
        inst->setVolume(1.0f);
        inst->stop();
    }

    if (playerVocals != nullptr)   { delete playerVocals;   playerVocals   = nullptr; }
    if (opponentVocals != nullptr) { delete opponentVocals; opponentVocals = nullptr; }

    if (!song.needsVoices) return;

    if (song.isVSlice) {
        auto resolveVoice = [&](const std::string& charId) -> flixel::FlxSound* {
            std::string id = charId;
            while (true) {
                flixel::FlxSound* s = tryLoadVoice(base + "Voices-" + id + soundExt);
                if (s) return s;
                size_t dash = id.rfind('-');
                if (dash == std::string::npos) break;
                id = id.substr(0, dash);
            }
            return tryLoadVoice(base + "Voices" + soundExt);
        };

        playerVocals   = resolveVoice(song.player1);
        opponentVocals = resolveVoice(song.player2);

        if (playerVocals)   { playerVocals->setChannel(0);   playerVocals->setVolume(1.0f); }
        if (opponentVocals) { opponentVocals->setChannel(2); opponentVocals->setVolume(1.0f); }

        if (!playerVocals && !opponentVocals)
            std::cerr << "[SongLoader] No V-Slice voice files found in: " << base << std::endl;
    } else {
        playerVocals = tryLoadVoice(base + "Voices" + soundExt);
        if (playerVocals) {
            playerVocals->setChannel(0);
            playerVocals->setVolume(1.0f);
        } else {
            std::cerr << "[SongLoader] No Voices.ogg found in: " << base << std::endl;
        }
    }
}

void SongLoader::parseSongPath(const std::string& dataPath,
                               std::string& songName,
                               std::string& folder,
                               std::string& baseSongName) {
    songName     = dataPath;
    folder       = dataPath;
    baseSongName = dataPath;

    for (const char* suf : {"-easy", "-hard"}) {
        size_t sufLen = strlen(suf);
        if (folder.size() >= sufLen && folder.substr(folder.size() - sufLen) == suf) {
            size_t dashPos = folder.rfind('-');
            if (dashPos != std::string::npos) {
                folder       = folder.substr(0, dashPos);
                baseSongName = folder;
            }
            break;
        }
    }
}
