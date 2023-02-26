#pragma once

#define TICK_FREQUENCY 0.2

#include "data.hpp"
#include <memory>

class Player;
class Level;

namespace raycaster
{
class Caster;
}

class AI
{
public:
    AI(Player& player, std::unique_ptr<raycaster::Caster>& caster);

    void registerSprites();

    bool killNpc(double x, double y);

    double distanceToNearestNpc();

    int tick(double frameTime, bool flashlight);

private:
    Player& player;
    std::unique_ptr<raycaster::Caster>& caster; // piękny design, nie ma co
};

