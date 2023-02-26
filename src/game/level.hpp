#pragma once

#include "data.hpp"

#include <array>
#include <string>

class Player;
namespace sdl
{
class Surface;
}

enum class BlockType
{
    None,
    Wall,
    Pillar
};

class Level
{
public:
    constexpr static auto arraySize = levelSize * levelSize;
    using TileArray = std::array<int, arraySize>;
    using VisitedArray = std::array<bool, arraySize>;

    int id;
    LevelInfo li{};

    TileArray map{};
    VisitedArray visited{};

    Items items{};
    NPCs npcs{};

    explicit Level(int id) : id(id) {}
    explicit Level(const std::string& filename);

    int at(int x, int y) const;
    void addItem(int x, int y, int id);
    BlockType blockType(int x, int y) const;
    sdl::Surface drawMap(const Player& player);

private:
    bool isKnownWall(int x, int y);
    int wallType(int x, int y);
};

