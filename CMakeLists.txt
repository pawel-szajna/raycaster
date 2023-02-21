cmake_minimum_required(VERSION 3.10)
project(raycaster)

set(CMAKE_CXX_STANDARD 23)

add_library(
    libraycaster
    ai.c
    caster.c
    data.c
    generator.c
    level.c
    player.c
    ui.c
)

add_subdirectory(sprig)

include_directories(
    .
    sprig/source
)

add_executable(
    raycaster
    main.c
)

target_link_libraries(
    raycaster
    libraycaster
    SDL
    SDL_ttf
    m
    sprig
)
