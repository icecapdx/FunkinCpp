#include "GameplayManager.h"
#include "song/Conductor.h"
#include <iostream>
#include <random>

GameplayManager::GameplayManager(NoteManager* noteManager, Strumline* opponentStrumline,
                                 Character* dad, Character* gf, HealthBar* healthBar,
                                 flixel::FlxSound* vocals, const SwagSong& song)
    : noteManager(noteManager)
    , opponentStrumline(opponentStrumline)
    , dad(dad)
    , gf(gf)
    , healthBar(healthBar)
    , vocals(vocals)
    , song(song)
    , missSound1(nullptr)
    , missSound2(nullptr)
    , missSound3(nullptr)
{
    std::random_device rd;
    rng = std::mt19937(rd());
    
    std::string soundExt = ".ogg";
    missSound1 = new flixel::FlxSound();
    missSound1->loadAsChunk(ASSETS_PATH "assets/sounds/missnote1" + soundExt, false, false);
    missSound2 = new flixel::FlxSound();
    missSound2->loadAsChunk(ASSETS_PATH "assets/sounds/missnote2" + soundExt, false, false);
    missSound3 = new flixel::FlxSound();
    missSound3->loadAsChunk(ASSETS_PATH "assets/sounds/missnote3" + soundExt, false, false);
}

GameplayManager::~GameplayManager() {
    if (missSound1) delete missSound1;
    if (missSound2) delete missSound2;
    if (missSound3) delete missSound3;
}

void GameplayManager::handleOpponentNotes(float deltaTime) {
    if (!noteManager) return;
    
    auto& notes = noteManager->getActiveNotes();
    for (auto note : notes) {
        if (note && !note->mustPress) {
            if (!note->wasGoodHit) {
                float timeDiff = note->strumTime - Conductor::songPosition;
                
                if (timeDiff <= 45.0f && timeDiff >= -Conductor::safeZoneOffset) {
                    note->canBeHit = true;
                    
                    if (vocals && song.needsVoices) {
                        vocals->setVolume(1.0f);
                    }
                    
                    if (opponentStrumline) {
                        opponentStrumline->playNote(note->noteData, true);
                    }
                    
                    if (dad) {
                        std::string animToPlay = "";
                        switch (note->noteData) {
                            case 0: animToPlay = "singLEFT"; break;
                            case 1: animToPlay = "singDOWN"; break;
                            case 2: animToPlay = "singUP"; break;
                            case 3: animToPlay = "singRIGHT"; break;
                        }
                        if (!animToPlay.empty()) {
                            dad->playAnim(animToPlay, true);
                            dad->holdTimer = 0.0f;
                        }
                    }
                    
                    note->wasGoodHit = true;
                    note->hitTime = Conductor::songPosition;
                    
                    if (note->sustainLength > 0) {
                        note->isHolding = true;
                    } else {
                        note->kill = true;
                    }
                }
            }
            else if (note->sustainLength > 0 && note->isHolding) {
                float sustainEndTime = note->strumTime + note->sustainLength;
                
                if (Conductor::songPosition < sustainEndTime) {
                    if (opponentStrumline) {
                        opponentStrumline->playNote(note->noteData, true);
                    }
                    if (dad) {
                        dad->holdTimer = 0.0f;
                    }
                } else {
                    note->isHolding = false;
                    note->kill = true;
                    if (opponentStrumline) {
                        opponentStrumline->playStatic(note->noteData);
                    }
                }
            }
        }
    }
}

void GameplayManager::noteMiss(int direction, Character* boyfriend, flixel::FlxSound* vocalsParam) {
    if (healthBar) {
        healthBar->setHealth(healthBar->getHealth() - 0.1f);
    }
    
    if (vocalsParam) {
        vocalsParam->setVolume(0.0f);
    }
    
    if (boyfriend && direction >= 0 && direction < 4) {
        std::string animToPlay = "";
        switch (direction) {
            case 0: animToPlay = "singLEFTmiss"; break;
            case 1: animToPlay = "singDOWNmiss"; break;
            case 2: animToPlay = "singUPmiss"; break;
            case 3: animToPlay = "singRIGHTmiss"; break;
        }
        if (!animToPlay.empty()) {
            boyfriend->playAnim(animToPlay, true);
            boyfriend->holdTimer = 0.0f;
        }
    }
    
    std::uniform_int_distribution<> missNoteDist(1, 3);
    std::uniform_real_distribution<> volumeDist(0.5, 0.8);
    
    int missNoteNum = missNoteDist(rng);
    float volume = static_cast<float>(volumeDist(rng));
    
    flixel::FlxSound* selectedSound = nullptr;
    switch(missNoteNum) {
        case 1: selectedSound = missSound1; break;
        case 2: selectedSound = missSound2; break;
        case 3: selectedSound = missSound3; break;
    }
    
    if (selectedSound) {
        selectedSound->setVolume(volume);
        selectedSound->play(true);
    }
}