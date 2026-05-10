if(NOT "$ENV{DEVKITPRO}" STREQUAL "")
    cmake_path(CONVERT "$ENV{DEVKITPRO}/portlibs/switch/bin" TO_CMAKE_PATH_LIST _dkp_pkg_hint)
    cmake_path(CONVERT "$ENV{DEVKITPRO}/tools/bin" TO_CMAKE_PATH_LIST _dkp_tools_hint)
else()
    set(_dkp_pkg_hint "")
    set(_dkp_tools_hint "")
endif()

find_program(PKG_CONFIG_EXECUTABLE
    NAMES aarch64-none-elf-pkg-config pkg-config
    HINTS "${_dkp_pkg_hint}"
    REQUIRED
)
find_program(NACPTOOL_EXE nacptool HINTS "${_dkp_tools_hint}" REQUIRED)
find_program(ELF2NRO_EXE elf2nro HINTS "${_dkp_tools_hint}" REQUIRED)

set(NX_APP_TITLE "Friday Night Funkin'" CACHE STRING "Application title (NACP)")
set(NX_APP_AUTHOR "Funkin' Crew and phlxsk8r" CACHE STRING "Application author (NACP)")
set(NX_APP_VERSION "1.0.0" CACHE STRING "Application version (NACP; max 15 characters)")
set(NX_TITLE_ID "" CACHE STRING "16-digit hex application title ID for NACP (or empty to omit)")
set(NX_ICON "${CMAKE_SOURCE_DIR}/Funkin.NX/romfs/icon.jpg" CACHE FILEPATH "JPEG icon embedded in the NRO")
set(NX_ROMFS_DIR "${CMAKE_SOURCE_DIR}/Funkin.NX/romfs" CACHE PATH "Directory packed as RomFS inside the NRO")

option(NX_WITH_NACP "Generate control.nacp and embed it in the NRO" ON)
option(NX_WITH_ICON "Pass --icon to elf2nro when NX_ICON exists" ON)
option(NX_WITH_ROMFS "Pass --romfsdir to elf2nro when NX_ROMFS_DIR is a directory" ON)

string(LENGTH "${NX_APP_VERSION}" _nx_ver_len)
if(_nx_ver_len GREATER 15)
    message(FATAL_ERROR "NX_APP_VERSION must be at most 15 characters (NACP field size).")
endif()

if(NOT NX_TITLE_ID STREQUAL "" AND NOT NX_TITLE_ID MATCHES "^[0-9A-Fa-f]{16}$")
    message(FATAL_ERROR "NX_TITLE_ID must be exactly 16 hexadecimal digits or empty (got '${NX_TITLE_ID}').")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUNKIN_SDL REQUIRED IMPORTED_TARGET
    sdl2
    SDL2_image
    SDL2_mixer
    SDL2_ttf
)

add_executable(Funkin ${FUNKIN_SOURCES})

target_compile_definitions(Funkin PRIVATE __SWITCH__)

target_include_directories(Funkin PRIVATE
    ${CMAKE_SOURCE_DIR}/Funkin
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/flixel
    ${CMAKE_SOURCE_DIR}/lib/json/include
    ${CMAKE_SOURCE_DIR}/lib/tinyxml2
)

target_link_libraries(Funkin PRIVATE PkgConfig::FUNKIN_SDL)

include("${CMAKE_SOURCE_DIR}/cmake/FunkinPostTarget.cmake")

set(_nx_nro "${CMAKE_CURRENT_BINARY_DIR}/Funkin.nro")
set(_nro_depends Funkin)
set(_nro_cmd "${ELF2NRO_EXE}" "$<TARGET_FILE:Funkin>" "${_nx_nro}")

if(NX_WITH_NACP)
    set(_nx_nacp "${CMAKE_CURRENT_BINARY_DIR}/Funkin.nacp")
    if(NX_TITLE_ID STREQUAL "")
        add_custom_command(
            OUTPUT "${_nx_nacp}"
            COMMAND "${NACPTOOL_EXE}" --create "${NX_APP_TITLE}" "${NX_APP_AUTHOR}" "${NX_APP_VERSION}" "${_nx_nacp}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${_nx_nacp}"
            COMMAND "${NACPTOOL_EXE}" --create "${NX_APP_TITLE}" "${NX_APP_AUTHOR}" "${NX_APP_VERSION}" "${_nx_nacp}" "--titleid=${NX_TITLE_ID}"
            VERBATIM
        )
    endif()
    list(APPEND _nro_depends "${_nx_nacp}")
    list(APPEND _nro_cmd "--nacp=${_nx_nacp}")
endif()

if(NX_WITH_ICON AND EXISTS "${NX_ICON}")
    list(APPEND _nro_cmd "--icon=${NX_ICON}")
endif()

if(NX_WITH_ROMFS AND IS_DIRECTORY "${NX_ROMFS_DIR}")
    list(APPEND _nro_cmd "--romfsdir=${NX_ROMFS_DIR}")
endif()

add_custom_command(
    OUTPUT "${_nx_nro}"
    COMMAND ${_nro_cmd}
    DEPENDS ${_nro_depends}
    VERBATIM
)

add_custom_target(Funkin_nro ALL DEPENDS "${_nx_nro}")
