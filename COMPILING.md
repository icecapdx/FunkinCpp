# Compiling FunkinCpp

0. Setup
    - Get cmake and a c++ compiler (get devkitpro if you're compiling for NX (Switch), and vitasdk if you're compiling for PSVita)
1. Cloning the Repository: Make sure when you clone, you clone the submodules to get the libraries (this will take a GOOD ASS BIT due to SDL_image, SDL_mixer, and SDL_ttf's submodules):
    - `git clone --recurse-submodules https://github.com/icecapdx/FunkinCpp.git`
2. Run `cd /path/to/where/you/cloned/FunkinCpp` to enter the cloned repos repository
3. Platform Build Setup (Compiling should be the same on most platforms but anyways)
    - Windows: 
        - `cmake -B build && cmake --build build`
    - Nintendo Switch: 
        - `cmake -B build && cmake --build build -DNX_BUILD=ON`
    - Linux: 
        - `cmake -B build && cmake --build build`
    - PSVita:
        - `cmake -B build && cmake --build build -DVITA_BUILD=ON`