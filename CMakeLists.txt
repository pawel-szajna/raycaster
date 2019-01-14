cmake_minimum_required(VERSION 3.10)
project(raycaster C)

set(CMAKE_C_STANDARD 11)

link_libraries(
        SDL
        SDL_ttf
        m
        sprig
)

include_directories(.)

add_executable(raycaster
        ai.c
        ai.h
        caster.c
        caster.h
        data.c
        data.h
        generator.c
        generator.h
        level.c
        level.h
        main.c
        player.c
        player.h
        ui.c
        ui.h)
