#pragma once

#include "data.hpp"
#include "player.hpp"

class Game
{
public:
    Game();
    ~Game();

    void work();

private:
    GameConfig config{};
    GameMode mode{GameMode::Initial};
    Player player{config};

    char texts[6][128];
};