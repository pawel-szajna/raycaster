#include "level.hpp"

#include <cstdio>
#include <cmath>
#include <spdlog/spdlog.h>

#include "ai.hpp"
#include "player.hpp"
#include "sdlwrapper/sdl.hpp"

Level::Level(const std::string& filename) : id(1)
{
    spdlog::info("Loading map {}...", filename);

    auto data = fopen(filename.c_str(), "rb");
    assert(data);
    fread((char*)(&li), sizeof(char), sizeof(LevelInfo) / sizeof(char), data);
    // fread((char*)(level), sizeof(char), (levelSize * levelSize * sizeof(int)) / sizeof(char), data);
//    npcs.clear();
//    for(auto x = 0; x < li->npcCount; ++x)
//    {
//        fread(&dx, sizeof(int), 1, data);
//        fread(&dy, sizeof(int), 1, data);
//        fread(&dz, sizeof(int), 1, data);
//        AddNPC(npcs, dx, dy, dz);
//    }
    fclose(data);
}

void Level::addItem(int x, int y, int num)
{
    spdlog::debug("Adding new item #{} [{};{}]", num, x, y);
    items.push_back(Item{x + 0.5, y + 0.5, 1, num});
}

int BlockType(int* level, int x, int y)
{
    return level[levelSize * x + y] % 16;
}

namespace
{
std::string playerArrow(double angle)
{
    if (angle < 0.39) return "gfx/map32.bmp";
    if (angle < 1.18) return "gfx/map31.bmp";
    if (angle < 1.96) return "gfx/map30.bmp";
    if (angle < 2.75) return "gfx/map29.bmp";
    if (angle < 3.53) return "gfx/map28.bmp";
    if (angle < 4.31) return "gfx/map27.bmp";
    if (angle < 5.11) return "gfx/map34.bmp";
    if (angle < 5.89) return "gfx/map33.bmp";
    return "gfx/map32.bmp";
}
}

bool isKnownWall(int* level, std::array<bool, 4096>& visited, int x, int y)
{
    if (x < 0 or y < 0 or x >= levelSize or y >= levelSize)
    {
        return false;
    }

    return not visited[levelSize * x + y] and (level[levelSize * x + y] % 16) == 1;
}

int wallType(int* level, std::array<bool, 4096>& visited, int x, int y)
{
    int wallType{1};

    if (isKnownWall(level, visited, x - 1, y)) wallType += 1;
    if (isKnownWall(level, visited, x, y - 1)) wallType += 2;
    if (isKnownWall(level, visited, x + 1, y)) wallType += 4;
    if (isKnownWall(level, visited, x, y + 1)) wallType += 8;

    return wallType;
}

sdl::Surface drawMap(int* level, Player& player)
{
    auto& visited = player.currentLevel().visited;
    const auto& position = player.getPosition();

    auto map = sdl::make_surface(levelSize * 8, levelSize * 8);

    for(int x = 0; x < levelSize; ++x) for(int y = 0; y < levelSize; ++y)
    {
        std::string texturePath{"gfx/map20.bmp"};
        if ((int)position.x == x and (int)position.y == y)
        {
            texturePath = playerArrow(atan2(position.dirY, position.dirX) + 3.1415);
        }
        else if (visited[levelSize * x + y])
        {
            switch((level[levelSize * x + y]) % 16)
            {
                case 1:
                    texturePath = std::format("gfx/map{:02}.bmp", wallType(level, visited, x, y));
                    break;
                case 2:
                    texturePath = "gfx/map19.bmp";
                    break;
                default:
                    texturePath = "gfx/map20.bmp";
            }
        }

        sdl::textures.get(texturePath).render(map, {static_cast<Sint16>(x * 8), static_cast<Sint16>(y * 8), 8, 8}, {0, 0, 8, 8});
    }

    return map;
}