#pragma once

#include "data.hpp"

#include <array>
#include <string>

class Player;
namespace sdl
{
class Surface;
}

class Level
{
public:
    int id;
    LevelInfo li{};
    std::array<int, levelSize * levelSize> level{};
    std::array<bool, levelSize * levelSize> visited{};

    Items items{};
    NPCs npcs{};

    explicit Level(int id) : id(id) {}
    explicit Level(const std::string& filename);

    void addItem(int x, int y, int id);
};

int BlockType(int* level, int x, int y);
void LoadLevel(int* level, LevelInfo* li, NPCs& npcs, const std::string& filename);
sdl::Surface drawMap(int* level, Player& player);
