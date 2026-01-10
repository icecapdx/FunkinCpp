#pragma once

#include "notes/NoteManager.h"
#include "notes/Strumline.h"
#include "character/Character.h"
#include "components/HealthBar.h"
#include "song/Song.h"
#include <flixel/sound/FlxSound.h>
#include <random>

class GameplayManager {
public:
    GameplayManager(NoteManager* noteManager, Strumline* opponentStrumline,
                   Character* dad, Character* gf, HealthBar* healthBar,
                   flixel::FlxSound* vocalsOpponent, const SwagSong& song);
    ~GameplayManager();
    
    void handleOpponentNotes(float deltaTime);
    void noteMiss(int direction, Character* boyfriend, flixel::FlxSound* vocalsPlayer);
    
private:
    NoteManager* noteManager;
    Strumline* opponentStrumline;
    Character* dad;
    Character* gf;
    HealthBar* healthBar;
    flixel::FlxSound* vocalsOpponent;
    SwagSong song;
    
    flixel::FlxSound* missSound1;
    flixel::FlxSound* missSound2;
    flixel::FlxSound* missSound3;
    
    std::mt19937 rng;
};