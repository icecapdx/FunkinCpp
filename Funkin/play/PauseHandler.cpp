#include "PauseHandler.h"
#include "../game/GameConfig.h"
#include "input/Controls.h"
#include <flixel/FlxG.h>
#include <SDL.h>
#include <iostream>

PauseHandler::PauseHandler()
    : wasPaused(false)
{
}

void PauseHandler::update(float elapsed, flixel::FlxSound* inst, 
                          flixel::FlxSound* vocalsPlayer, flixel::FlxSound* vocalsOpponent,
                          float& songPosition, unsigned int& musicStartTicks,
                          flixel::FlxSubState*& subState,
                          std::function<void(flixel::FlxSubState*)> openSubStateFunc,
                          std::function<void()> closeSubStateFunc) {
    bool justClosed = (wasPaused && !subState);
    
    if (justClosed) {
        if (musicStartTicks > 0) {
            musicStartTicks = SDL_GetTicks() - static_cast<unsigned int>(songPosition);
        }
        
        if (inst && inst->paused) {
            inst->resume();
        }
        if (vocalsPlayer && vocalsPlayer->paused) {
            vocalsPlayer->resume();
        }
        if (vocalsOpponent && vocalsOpponent->paused) {
            vocalsOpponent->resume();
        }
    }
    
    wasPaused = (subState != nullptr);

    Controls* controls = GameConfig::getInstance()->controls;
    bool pausePressed = controls->justPressedAction(ControlAction::PAUSE);
    
    if (pausePressed && !subState && !justClosed) {
        if (inst) {
            inst->pause();
        }
        if (vocalsPlayer) {
            vocalsPlayer->pause();
        }
        if (vocalsOpponent) {
            vocalsOpponent->pause();
        }
        
        if (musicStartTicks > 0) {
            musicStartTicks = SDL_GetTicks() - static_cast<unsigned int>(songPosition);
        }
        
        PauseSubState* pauseSubState = new PauseSubState();
        openSubStateFunc(pauseSubState);
    }
}