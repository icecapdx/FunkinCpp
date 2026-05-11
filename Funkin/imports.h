// Makes handling assets easier on vita lol
#ifdef __vita__
    #define ASSETS_PATH "ux0:data/Funkin/"
#else
    #define ASSETS_PATH ""
#endif

#include <memory>
#include <string>
#include <iostream>
#include <SDL_ttf.h>
#include <vector>