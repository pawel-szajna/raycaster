#pragma once

#include "level.hpp"
#include "ai.hpp"

#include <random>

class Generator
{
public:
    explicit Generator(Level& level);

    void fillMap(int playerX, int playerY, bool bonusRoom);
    NPCs generateNpcs(AI& ai);

private:
    int& at(int x, int y);
    bool wallAt(int x, int y);

    void clearLevel();
    void fixWallTextures();

    void spawnBonusRoom();
    void spawnCorridor();
    void spawnRandomRoom();

    void dig(int currentX, int currentY, int connectionX, int connectionY, bool fill);

    Level& level;
    Level::TileArray& map;

    std::random_device rd;
    std::mt19937 rng{rd()};
};
