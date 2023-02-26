cmake_minimum_required(VERSION 3.10)
project(raycaster)

set(CMAKE_CXX_STANDARD 23)

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

add_compile_definitions(SPDLOG_USE_STD_FORMAT)

add_library(
    sdlwrapper
    src/sdlwrapper/sdl.cpp
)

target_include_directories(
    sdlwrapper
    PRIVATE
    src/sdlwrapper
)

target_compile_options(
    sdlwrapper
    PRIVATE
    -Wall -pedantic -O3
)

add_library(
    raycaster
    src/raycaster/caster.cpp
)

target_include_directories(
    raycaster
    PRIVATE
    src/raycaster
)

target_compile_options(
    raycaster
    PRIVATE
    -Wall -pedantic -O3
)

add_library(
    game
    src/game/ai.cpp
    src/game/data.cpp
    src/game/generator.cpp
    src/game/level.cpp
    src/game/player.cpp
    src/game/scripting.cpp
    src/game/ui.cpp
    src/game/game.cpp
    src/game/gameplay.cpp
)

target_include_directories(
    game
    PRIVATE
    src/game
)

target_compile_options(
    game
    PRIVATE
    -Wall -pedantic -O0
)

add_subdirectory(sprig)
target_compile_options(sprig PUBLIC -Wno-narrowing -Wno-misleading-indentation)

include_directories(
    src
    sprig/source
    /usr/include/lua5.3
    sol2/include
)

add_executable(
    demo
    src/main.cpp
)

target_compile_options(
    demo
    PUBLIC
    -Wall -pedantic -O3
)

target_link_libraries(
    demo
    game
    raycaster
    sdlwrapper
    SDL
    SDL_ttf
    m
    tbb
    sprig
    spdlog::spdlog
    lua5.3
)
