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

include("${CMAKE_SOURCE_DIR}/cmake/FunkinSources.cmake")

option(SHOW_CONSOLE "Show console window for debugging (Windows desktop only)" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Switch" OR CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    include("${CMAKE_SOURCE_DIR}/Funkin.NX/Switch.cmake")
elseif(VITA OR BUILD_VITA OR CMAKE_SYSTEM_NAME STREQUAL "Vita")
    include("${CMAKE_SOURCE_DIR}/Funkin.Vita/Vita.cmake")
else()
    include("${CMAKE_SOURCE_DIR}/cmake/Desktop.cmake")
endif()
