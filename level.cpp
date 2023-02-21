#include "level.hpp"

#include <cstdio>
#include <cmath>
#include <spdlog/spdlog.h>

#include "ai.hpp"
#include "player.hpp"
#include "sdl.hpp"

int BlockType(int* level, int x, int y)
{
    return level[levelSize * x + y] % 16;
}

void LoadLevel(int* level, LevelInfo* li, NPCs& npcs, const char* filename)
{
    int x;
    int dx, dy, dz;
    FILE* data;

    spdlog::info("Loading map {}...", filename);
    data = fopen(filename, "rb");
    assert(data);
    fread((char*)(li), sizeof(char), sizeof(LevelInfo) / sizeof(char), data);
    fread((char*)(level), sizeof(char), (levelSize * levelSize * sizeof(int)) / sizeof(char), data);
    npcs.clear();
    for(x = 0; x < li->npcCount; ++x)
    {
        fread(&dx, sizeof(int), 1, data);
        fread(&dy, sizeof(int), 1, data);
        fread(&dz, sizeof(int), 1, data);
        AddNPC(npcs, dx, dy, dz);
    }
    fclose(data);
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

int isitwall(int* level, char* visited, int x, int y)
{
    if(x < 0 || y < 0 || x >= levelSize || y >= levelSize) return 0;
    if(!visited[levelSize * x + y]) return 0;
    if(level[levelSize * x + y] % 16 == 1) return 1;
    return 0;
}

int walltype(int* level, char* visited, int x, int y)
{
    int a = 0;

    if(isitwall(level, visited, x - 1, y)) a += 1;
    if(isitwall(level, visited, x, y - 1)) a += 2;
    if(isitwall(level, visited, x + 1, y)) a += 4;
    if(isitwall(level, visited, x, y + 1)) a += 8;

    return 1 + a;
}

sdl::Surface drawMap(int* level, Player& player)
{
    char* visited = player.currentLevel().visited;
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
                    char filename[64];
                    sprintf(filename, "gfx/map%02d.bmp", walltype(level, visited, x, y));
                    texturePath = filename;
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