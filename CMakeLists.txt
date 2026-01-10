cmake_minimum_required(VERSION 3.30)

project(Funkin
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

# SDL2
set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(LIBC ON CACHE INTERNAL "" FORCE)
endif()
add_subdirectory("lib/SDL" EXCLUDE_FROM_ALL)

set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("lib/SDL_image" EXCLUDE_FROM_ALL)

set(SDL2MIXER_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/SDL_mixer" EXCLUDE_FROM_ALL)

set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/SDL_ttf" EXCLUDE_FROM_ALL)

add_executable(Funkin
    # funkin
    Funkin/main.cpp
    Funkin/FunkinState.cpp

    # game
    Funkin/game/GameConfig.cpp
    Funkin/game/Section.cpp

    # play/character
    Funkin/play/character/Character.cpp
    Funkin/play/character/CharacterManager.cpp

    # play/components
    Funkin/play/components/HealthBar.cpp
    Funkin/play/components/HealthIcon.cpp
    Funkin/play/components/PopUpStuff.cpp

    # play/input
    Funkin/play/input/Controls.cpp

    # play/notes
    Funkin/play/notes/NoteHitHandler.cpp
    Funkin/play/notes/NoteManager.cpp
    Funkin/play/notes/NoteSprite.cpp
    Funkin/play/notes/NoteUpdateHandler.cpp
    Funkin/play/notes/Strumline.cpp

    # play/scoring
    Funkin/play/scoring/Highscore.cpp
    Funkin/play/scoring/Scoring.cpp

    # play/song
    Funkin/play/song/Conductor.cpp
    Funkin/play/song/Song.cpp
    Funkin/play/song/SongLoader.cpp

    # play/stage
    Funkin/play/stage/Stage.cpp

    # play
    Funkin/play/CameraManager.cpp
    Funkin/play/Countdown.cpp
    Funkin/play/GameOverSubState.cpp
    Funkin/play/GameplayManager.cpp
    Funkin/play/PauseHandler.cpp
    Funkin/play/PauseSubState.cpp
    Funkin/play/PlayState.cpp
    Funkin/play/PlayStateRenderer.cpp

    # scripting
    Funkin/scripting/Script.cpp
    Funkin/scripting/ScriptAPI.cpp
    Funkin/scripting/ScriptManager.cpp

    # ui/components
    Funkin/ui/components/Alphabet.cpp
    Funkin/ui/components/AtlasMenuItem.cpp
    Funkin/ui/components/CheckBox.cpp
    Funkin/ui/components/MenuCharacter.cpp
    Funkin/ui/components/MenuItem.cpp

    # ui/freeplay
    Funkin/ui/freeplay/Capsule.cpp
    Funkin/ui/freeplay/DigitDisplay.cpp

    # ui
    Funkin/ui/CreditsState.cpp
    Funkin/ui/MainMenuState.cpp
    Funkin/ui/NewFreeplayState.cpp
    Funkin/ui/OptionsState.cpp
    Funkin/ui/StoryMenuState.cpp
    Funkin/ui/TitleState.cpp

    # libraries
    lib/tinyxml2/tinyxml2.cpp
    
    # neoflixel
    lib/flixel/addons/transition/FlxTransitionEffect.cpp
    lib/flixel/addons/FlxBackdrop.cpp
    lib/flixel/animation/FlxAnimationController.cpp
    lib/flixel/effects/FlxFlicker.cpp
    lib/flixel/graphics/frames/FlxAtlasFrames.cpp
    lib/flixel/input/FlxGamepad.cpp
    lib/flixel/sound/FlxSound.cpp
    lib/flixel/sound/FlxSoundGroup.cpp
    lib/flixel/text/FlxBitmapText.cpp
    lib/flixel/text/FlxText.cpp
    lib/flixel/tweens/FlxTween.cpp
    lib/flixel/util/FlxTimer.cpp
    lib/flixel/FlxBasic.cpp
    lib/flixel/FlxCamera.cpp
    lib/flixel/FlxG.cpp
    lib/flixel/FlxGame.cpp
    lib/flixel/FlxObject.cpp
    lib/flixel/FlxSprite.cpp
    lib/flixel/FlxState.cpp
    lib/flixel/FlxSubState.cpp
)

target_include_directories(Funkin PRIVATE 
    ${CMAKE_SOURCE_DIR}/Funkin
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/flixel
    ${CMAKE_SOURCE_DIR}/lib/json/include
    ${CMAKE_SOURCE_DIR}/lib/tinyxml2
    ${CMAKE_SOURCE_DIR}/lib/Tsukiyo/include
    ${CMAKE_SOURCE_DIR}/lib/SDL/include
    ${CMAKE_SOURCE_DIR}/lib/SDL_image/include
    ${CMAKE_SOURCE_DIR}/lib/SDL_mixer/include
    ${CMAKE_SOURCE_DIR}/lib/SDL_ttf/include
)
target_link_libraries(Funkin PRIVATE SDL2main SDL2-static SDL2_image SDL2_mixer SDL2_ttf)

option(SHOW_CONSOLE "Show console window for debugging" ON)

if(WIN32)
    if(SHOW_CONSOLE)
        set_target_properties(Funkin PROPERTIES WIN32_EXECUTABLE FALSE)
    else()
        set_target_properties(Funkin PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

set_target_properties(Funkin PROPERTIES DEBUG_POSTFIX "_debug")