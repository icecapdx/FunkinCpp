#pragma once

#include <flixel/sound/FlxSound.h>
#include <flixel/FlxSubState.h>
#include "PauseSubState.h"
#include <functional>

class PauseHandler {
public:
    PauseHandler();
    
    void update(float elapsed,
                flixel::FlxSound* inst,
                flixel::FlxSound* vocals,
                flixel::FlxSound* opponentVocals,
                float& songPosition, unsigned int& musicStartTicks,
                flixel::FlxSubState*& subState,
                std::function<void(flixel::FlxSubState*)> openSubStateFunc,
                std::function<void()> closeSubStateFunc);
    
private:
    bool wasPaused;
};