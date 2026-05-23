#pragma once

#include "Song.h"
#include <string>
#include <flixel/sound/FlxSound.h>

class SongLoader {
public:
    static SwagSong loadSong(const std::string& dataPath);
    static void loadSongAudio(const std::string& songName,
                              flixel::FlxSound*& inst,
                              flixel::FlxSound*& playerVocals,
                              flixel::FlxSound*& opponentVocals,
                              const SwagSong& song);

private:
    static void parseSongPath(const std::string& dataPath, std::string& songName, std::string& folder, std::string& baseSongName);
    static flixel::FlxSound* tryLoadVoice(const std::string& path);
};
