cmake_minimum_required(VERSION 3.10)
project(raycaster)

set(CMAKE_CXX_STANDARD 23)

add_library(
    libraycaster
    ai.cpp
    caster.cpp
    data.cpp
    generator.cpp
    level.cpp
    player.cpp
    ui.cpp
)

add_subdirectory(sprig)

include_directories(
    .
    sprig/source
)

add_executable(
    raycaster
    main.cpp
)

target_link_libraries(
    raycaster
    libraycaster
    SDL
    SDL_ttf
    m
    sprig
)
