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

target_compile_options(libraycaster PRIVATE -Wall -pedantic)

add_subdirectory(sprig)

target_compile_options(sprig PUBLIC -Wno-narrowing -Wno-misleading-indentation)

include_directories(
    .
    sprig/source
)

add_executable(
    raycaster
    main.cpp
)

target_compile_options(raycaster PUBLIC -Wall -pedantic)

target_link_libraries(
    raycaster
    libraycaster
    SDL
    SDL_ttf
    m
    sprig
)
