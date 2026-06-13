find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(Funkin ${FUNKIN_SOURCES})

target_include_directories(Funkin PRIVATE
    ${CMAKE_SOURCE_DIR}/Funkin
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/flixel
    ${CMAKE_SOURCE_DIR}/lib/json/include
    ${CMAKE_SOURCE_DIR}/lib/tinyxml2
    ${CMAKE_SOURCE_DIR}/lib/tsukiyo/include
)

target_link_libraries(Funkin PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)

if(WIN32)
    if(SHOW_CONSOLE)
        set_target_properties(Funkin PROPERTIES WIN32_EXECUTABLE FALSE)
    else()
        set_target_properties(Funkin PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

include("${CMAKE_SOURCE_DIR}/cmake/FunkinPostTarget.cmake")
