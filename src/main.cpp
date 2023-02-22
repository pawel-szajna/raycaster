#include "game/game.hpp"
#include <spdlog/spdlog.h>

#ifndef NDEBUG
#undef main
#endif

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Raycaster demo, version 2023");

    Game game;
    game.work();
}