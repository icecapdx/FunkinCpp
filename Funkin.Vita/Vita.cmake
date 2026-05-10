if(NOT VITASDK OR VITASDK STREQUAL "")
    if(DEFINED ENV{VITASDK} AND NOT "$ENV{VITASDK}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{VITASDK}" VITASDK)
        set(VITASDK "${VITASDK}" CACHE PATH "PlayStation Vita SDK root")
    endif()
endif()

if(NOT VITASDK OR VITASDK STREQUAL "")
    message(FATAL_ERROR "VITASDK is not set. Export VITASDK or pass it to CMake.")
endif()

include("${VITASDK}/share/vita.cmake" REQUIRED)

# App metadata
set(VITA_APP_NAME "Friday Night Funkin'" CACHE STRING "LiveArea title")
set(VITA_TITLEID "FNFN00001" CACHE STRING "9-character Title ID")
set(VITA_VERSION "01.00" CACHE STRING "Version string")

string(LENGTH "${VITA_TITLEID}" _vita_tid_len)
if(NOT _vita_tid_len EQUAL 9)
    message(FATAL_ERROR "VITA_TITLEID must be exactly 9 characters.")
endif()

cmake_path(CONVERT "${VITASDK}/bin" TO_CMAKE_PATH_LIST _vita_pkg_hint)

find_program(PKG_CONFIG_EXECUTABLE
    NAMES arm-vita-eabi-pkg-config pkg-config
    HINTS "${_vita_pkg_hint}"
    REQUIRED
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FUNKIN_SDL REQUIRED IMPORTED_TARGET
    sdl2
    SDL2_image
    SDL2_mixer
    SDL2_ttf
)

add_executable(Funkin ${FUNKIN_SOURCES})

set_target_properties(Funkin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(Funkin PRIVATE
    ${CMAKE_SOURCE_DIR}/Funkin
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/flixel
    ${CMAKE_SOURCE_DIR}/lib/json/include
    ${CMAKE_SOURCE_DIR}/lib/tinyxml2
)

target_compile_definitions(Funkin PRIVATE
    __vita__
)

target_link_libraries(Funkin PRIVATE
    PkgConfig::FUNKIN_SDL

    -Wl,--start-group
    vitaGL
    vitashark
    SceShaccCg_stub
    -Wl,--end-group

    -Wl,--whole-archive
    ${VITASDK}/arm-vita-eabi/lib/libSceShaccCgExt.a
    -Wl,--no-whole-archive

    taihen_stub

    SceDisplay_stub
    SceGxm_stub
    SceCtrl_stub
    SceTouch_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceCommonDialog_stub
    ScePower_stub
    SceKernelDmacMgr_stub
    SceSysmodule_stub
    ScePgf_stub
    ScePvf_stub
    m
)

set(_vita_sce "${CMAKE_SOURCE_DIR}/Funkin.Vita/sce_sys")

vita_create_self(Funkin.self Funkin UNSAFE)

vita_create_vpk(Funkin.vpk "${VITA_TITLEID}" Funkin.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}

    FILE "${_vita_sce}/icon0.png" sce_sys/icon0.png
    FILE "${_vita_sce}/livearea/contents/template.xml" sce_sys/livearea/contents/template.xml
    FILE "${_vita_sce}/livearea/contents/bg0.png" sce_sys/livearea/contents/bg0.png
    FILE "${_vita_sce}/livearea/contents/startup.png" sce_sys/livearea/contents/startup.png
)

include("${CMAKE_SOURCE_DIR}/cmake/FunkinPostTarget.cmake")

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20.0")
    add_dependencies(Funkin.vpk-vpk Funkin.self-self)
endif()