#include <flixel/FlxGame.h>
#include "ui/TitleState.h"
#include "imports.h"

#ifdef __vita__
unsigned int _sceUserMainThreadStackSize = 0x200000;   // 2MB stack
unsigned int _sceUserMainThreadHeapSize  = 0x8000000;  // 128MB heap
#endif

struct Config {
    int width = 1280;        // Width of the game in pixels
    int height = 720;        // Height of the game in pixels
    int framerate = 60;      // How many frames per second the game should run at
    bool skipSplash = false; // Whether to skip the splash screen (DOES NOTHING AS OF RN LOL)
    bool startFullscreen = false; // Whether to start in fullscreen
    std::string title = "Friday Night Funkin'"; // Window title
};

int main(int argc, char* argv[]) {
    Config config;

    try {
        flixel::FlxGame game(
            config.width,
            config.height,
            config.framerate,
            config.framerate,
            config.title
        );

        game.switchState(new TitleState());

        if (config.startFullscreen) {
            game.setFullscreen(true);
        }

        game.run();
    }
    catch (const std::exception& e) {
        fprintf(stderr, "[FATAL] %s\n", e.what());
        fflush(stderr);
        return 1;
    }

    return 0;
}