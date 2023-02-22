#pragma once

#include "data.hpp"
#include "player.hpp"
#include <memory>

namespace raycaster
{
class Caster;
}

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
    std::unique_ptr<raycaster::Caster> caster{};

    char texts[6][128];
};