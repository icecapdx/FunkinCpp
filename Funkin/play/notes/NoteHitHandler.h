#pragma once

#include "NoteSprite.h"
#include "NoteManager.h"
#include "Strumline.h"
#include "../input/Controls.h"
#include "../character/Character.h"
#include "../scoring/Scoring.h"
#include "../components/PopUpStuff.h"
#include "../components/HealthBar.h"
#include <vector>
#include <random>
#include <flixel/sound/FlxSound.h>
#include <flixel/text/FlxText.h>

class NoteHitHandler {
public:
    NoteHitHandler(Controls* controls, NoteManager* noteManager, 
                   Strumline* playerStrumline, Character* boyfriend,
                   HealthBar* healthBar, PopUpStuff* popUpStuff,
                   flixel::FlxText* scoreText, flixel::FlxSound* vocalsPlayer,
                   flixel::FlxCamera* camHUD);
    ~NoteHitHandler();
    
    void handleInput();
    
    int getScore() const { return score; }
    int getMisses() const { return misses; }
    int getCombo() const { return combo; }
    int getSicks() const { return sicks; }
    int getGoods() const { return goods; }
    int getBads() const { return bads; }
    int getShits() const { return shits; }
    
    void incrementScore(int amount) { score += amount; }
    void incrementMisses() { misses++; }
    void resetCombo() { combo = 0; }
    void updateScore() { updateScoreText(); }
    
    void setScore(int value) { score = value; }
    void setMisses(int value) { misses = value; }
    void setCombo(int value) { combo = value; }
    void decrementScore(int amount) { score -= amount; if (score < 0) score = 0; }    
    void setSicks(int value) { sicks = value; }
    void setGoods(int value) { goods = value; }
    void setBads(int value) { bads = value; }
    void setShits(int value) { shits = value; }
    
private:
    Controls* controls;
    NoteManager* noteManager;
    Strumline* playerStrumline;
    Character* boyfriend;
    HealthBar* healthBar;
    PopUpStuff* popUpStuff;
    flixel::FlxText* scoreText;
    flixel::FlxSound* vocalsPlayer;
    flixel::FlxCamera* camHUD;
    
    int score;
    int misses;
    int combo;
    int sicks;
    int goods;
    int bads;
    int shits;
    
    flixel::FlxSound* missSound1;
    flixel::FlxSound* missSound2;
    flixel::FlxSound* missSound3;
    std::mt19937 rng;
    
    void goodNoteHit(NoteSprite* note);
    void updateScoreText();
    void playMissSound();
};