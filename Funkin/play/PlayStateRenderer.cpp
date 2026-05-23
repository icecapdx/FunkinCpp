#include "PlayStateRenderer.h"

void PlayStateRenderer::draw(Stage* stage, Character* gf, Character* dad, Character* boyfriend,
                              Strumline* opponentStrumline, Strumline* playerStrumline,
                              NoteManager* noteManager, HealthBar* healthBar,
                              flixel::FlxText* scoreText, PopUpStuff* popUpStuff,
                              Countdown* countdown, flixel::FlxSubState* subState) {
    if (stage) {
        for (auto sprite : stage->getSprites()) {
            if (sprite && sprite->visible) {
                sprite->draw();
            }
        }
    }
    
    if (gf && gf->visible) {
        gf->draw();
    }
    if (dad && dad->visible) {
        dad->draw();
    }
    if (boyfriend && boyfriend->visible) {
        boyfriend->draw();
    }
    
    if (opponentStrumline) {
        opponentStrumline->draw();
    }
    if (playerStrumline) {
        playerStrumline->draw();
    }

    if (noteManager) {
        auto& notes = noteManager->getActiveNotes();
        for (auto note : notes) {
            if (note && note->visible) {
                note->draw();
            }
        }
        auto& animatingNotes = noteManager->getAnimatingNotes();
        for (auto note : animatingNotes) {
            if (note && note->visible) {
                note->draw();
            }
        }
    }

    if (healthBar) {
        healthBar->draw();
    }

    if (scoreText) {
        scoreText->draw();
    }
    
    if (popUpStuff) {
        popUpStuff->draw();
    }
    
    if (countdown) {
        flixel::FlxSprite* countdownSprite = countdown->getCurrentSprite();
        if (countdownSprite && countdownSprite->visible) {
            countdownSprite->draw();
        }
    }

    if (subState) {
        subState->draw();
    }
}