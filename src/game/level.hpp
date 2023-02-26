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

    class BlockInfo
    {
    public:
        // at(x, y) = 1 + (south << 4) + (east << 8) + (north << 12) + (west << 16);
        BlockInfo(int x, int y, Level::TileArray& map) : x(x), y(y), tile(map[x * levelSize + y]) {}

        int type() const { return tile % 16; }
        int textureSouth() const { return (tile >> 4) % 16; }
        int textureEast() const { return (tile >> 8) % 16; }
        int textureNorth() const { return (tile >> 12) % 16; }
        int textureWest() const { return (tile >> 16) % 16; }

    private:
        int x;
        int y;
        int& tile;
    };

    using BlockArray = std::vector<BlockInfo>;

    int id;
    LevelInfo li{};

    TileArray map{};
    VisitedArray visited{};
    BlockArray blocks;

    Items items{};
    NPCs npcs{};

    explicit Level(const std::string& filename);

    int at(int x, int y) const;
    void addItem(int x, int y, int id, const std::string& callback);
    BlockType blockType(int x, int y) const;
    sdl::Surface drawMap(const Player& player);

private:
    bool isKnownWall(int x, int y);
    int wallType(int x, int y);
};

