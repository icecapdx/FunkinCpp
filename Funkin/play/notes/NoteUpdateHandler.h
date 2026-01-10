#pragma once

#include "NoteManager.h"
#include "../character/Character.h"
#include "../GameplayManager.h"
#include "NoteHitHandler.h"
#include <flixel/sound/FlxSound.h>

class NoteUpdateHandler {
public:
    NoteUpdateHandler(NoteManager* noteManager, NoteHitHandler* noteHitHandler, 
                      GameplayManager* gameplayManager, Character* gf);
    
    void updateNotes(float elapsed, Character* boyfriend, flixel::FlxSound* vocalsPlayer);
    
private:
    NoteManager* noteManager;
    NoteHitHandler* noteHitHandler;
    GameplayManager* gameplayManager;
    Character* gf;
};