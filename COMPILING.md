# Compiling FunkinCpp

0. Setup
    - Get cmake and a c++ compiler (get devkitpro if you're compiling for NX (Switch), and vitasdk if you're compiling for PSVita)
1. Cloning the Repository: Make sure when you clone, you clone the submodules to get the libraries:
    - `git clone --recurse-submodules https://github.com/icecapdx/FunkinCpp.git`
2.
    - Install SDL2, SDL_mixer, SDL_image, and SDL_ttf
3. Run `cd /path/to/where/you/cloned/FunkinCpp` to enter the cloned repos repository
4. Platform Build Setup (Compiling should be the same on most platforms but anyways)
    - Desktop: 
        - `cmake -S . -B build && cmake --build build -j$(nproc)`
    - Nintendo Switch:
        - `cmake -S . -B build-nx -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" && cmake --build build-nx -j$(nproc)`
    - PSVita (vitasdk; requires `VITASDK`):
        - `cmake -S . -B build-vita -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" && cmake --build build-vita -j$(nproc)`
