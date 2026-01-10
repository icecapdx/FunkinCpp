#include "NoteManager.h"
#include "../../game/GameConfig.h"
#include <flixel/FlxG.h>
#include <flixel/tweens/FlxTweenUtil.h>
#include <iostream>
#include <algorithm>
#include <cmath>

std::vector<NoteSprite*> NoteManager::notePool;

NoteManager::NoteManager(flixel::FlxCamera* hudCamera)
    : hudCamera(hudCamera)
{
}

NoteManager::~NoteManager() {
    for (auto note : activeNotes) {
        returnToPool(note);
    }
    activeNotes.clear();
    
    for (auto note : unspawnedNotes) {
        returnToPool(note);
    }
    unspawnedNotes.clear();
    
    cachedNotes.clear();
}

void NoteManager::generateNotes(const SwagSong& song) {
    clear();
    cachedNotes.clear();
    
    if (song.notes.empty()) {
        return;
    }
    
    std::cout << "Generating notes from " << song.notes.size() << " sections" << std::endl;
    
    int totalNotes = 0;
    int currentSection = 0;
    
    for (const auto& section : song.notes) {
        for (const auto& noteData : section.sectionNotes) {
            if (noteData.size() >= 2) {
                float strumTime = noteData[0];
                int noteType = static_cast<int>(noteData[1]);
                
                if (noteType < 0 || noteType > 7) {
                    std::cout << "Warning: Invalid note type " << noteType << " in section " << currentSection << std::endl;
                    continue;
                }
                
                bool mustPress = section.mustHitSection;
                if (noteType >= 4) {
                    mustPress = !section.mustHitSection;
                    noteType = noteType % 4;
                }
                
                float sustainLength = (noteData.size() > 2) ? noteData[2] : 0;
                
                cachedNotes.push_back({strumTime, noteType, sustainLength, mustPress});
                
                NoteSprite* note = getPooledNote(strumTime, noteType, sustainLength, mustPress);
                unspawnedNotes.push_back(note);
                totalNotes++;
            }
        }
        currentSection++;
    }
    
    std::sort(cachedNotes.begin(), cachedNotes.end(),
        [](const CachedNoteData& a, const CachedNoteData& b) {
            return a.strumTime < b.strumTime;
        });
    
    std::sort(unspawnedNotes.begin(), unspawnedNotes.end(),
        [](NoteSprite* a, NoteSprite* b) {
            return a->strumTime < b->strumTime;
        });
    
    std::cout << "Generated " << totalNotes << " notes" << std::endl;
}

void NoteManager::regenerateFromCache() {
    for (auto note : activeNotes) {
        returnToPool(note);
    }
    activeNotes.clear();
    
    for (auto note : unspawnedNotes) {
        returnToPool(note);
    }
    unspawnedNotes.clear();
    
    if (cachedNotes.empty()) {
        std::cout << "No cached notes to regenerate from" << std::endl;
        return;
    }
    
    std::cout << "Regenerating " << cachedNotes.size() << " notes from cache (pool size: " << notePool.size() << ")" << std::endl;
    
    for (const auto& cached : cachedNotes) {
        NoteSprite* note = getPooledNote(cached.strumTime, cached.noteType, cached.sustainLength, cached.mustPress);
        unspawnedNotes.push_back(note);
    }
    
    std::cout << "Regenerated " << unspawnedNotes.size() << " notes from cache" << std::endl;
}

NoteSprite* NoteManager::getPooledNote(float strumTime, int noteType, float sustainLength, bool mustPress) {
    NoteSprite* note = nullptr;
    
    if (!notePool.empty()) {
        note = notePool.back();
        notePool.pop_back();
        note->reset(strumTime, noteType, sustainLength, mustPress);
        if (hudCamera) {
            note->camera = hudCamera;
        }
    } else {
        note = createNote(strumTime, noteType, sustainLength, mustPress);
    }
    
    return note;
}

void NoteManager::returnToPool(NoteSprite* note, bool hideImmediately) {
    if (note) {
        if (hideImmediately) {
            note->visible = false;
        }
        notePool.push_back(note);
    }
}

NoteSprite* NoteManager::createNote(float strumTime, int noteType, float sustainLength, bool mustPress) {
    NoteSprite* note = new NoteSprite(strumTime, noteType);
    note->sustainLength = sustainLength;
    note->mustPress = mustPress;
    note->scrollFactor.x = 0.0f;
    note->scrollFactor.y = 0.0f;
    
    if (mustPress) {
        note->x += static_cast<float>(flixel::FlxG::width) / 2.0f;
    }
    
    if (hudCamera) {
        note->camera = hudCamera;
    }
    
    return note;
}

void NoteManager::updateSpawning(float songPosition) {
    while (!unspawnedNotes.empty()) {
        NoteSprite* nextNote = unspawnedNotes[0];
        if (nextNote->strumTime - songPosition > 1500) {
            break;
        }
        
        activeNotes.push_back(nextNote);
        unspawnedNotes.erase(unspawnedNotes.begin());
    }
}

void NoteManager::clear() {
    for (auto note : activeNotes) {
        returnToPool(note);
    }
    activeNotes.clear();
    
    for (auto note : unspawnedNotes) {
        returnToPool(note);
    }
    unspawnedNotes.clear();
}

void NoteManager::clearWithoutPooling() {
    activeNotes.clear();
    unspawnedNotes.clear();
}

void NoteManager::animateNotesDownward() {
    float screenHeight = static_cast<float>(flixel::FlxG::height);
    float duration = 0.5f;
    bool isDownscroll = GameConfig::getInstance()->isDownscroll();
    
    animatingNotes.clear();
    for (auto note : activeNotes) {
        if (note && note->visible) {
            animatingNotes.push_back(note);
        }
    }
    
    // me when i have too mny notes so igita kill dem all
    if (animatingNotes.size() > 100) {
        for (auto note : animatingNotes) {
            if (note) {
                note->visible = false;
            }
        }
        animatingNotes.clear();
        return;
    }
    
    for (auto note : animatingNotes) {
        if (!note) continue;
        float targetY = isDownscroll ? (0.0f - note->height) : (screenHeight + note->y);
        auto cleanupCallback = [note]() {
            if (note) {
                note->visible = false;
            }
        };
        flixel::tweens::tweenY(note, targetY, duration, flixel::tweens::FlxEase::expoIn, cleanupCallback);
    }
}