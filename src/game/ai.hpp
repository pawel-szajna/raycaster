#pragma once

#define TICK_FREQUENCY 0.2

#include "data.hpp"

class Player;
class Level;

class AI
{
public:
    AI(Player& player);

    void addNpc(double x, double y, int firstTexture);
    bool killNpc(double x, double y);

    double distanceToNearestNpc();

    int tick(double frameTime, bool flashlight);

private:
    Player& player;
};

