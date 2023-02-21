#pragma once

/* uncomment for release */
/*#define NDEBUG*/

#include "SDL/SDL.h"
#include <cassert>
#include <vector>
#include <algorithm>
#include <unordered_map>

constexpr auto speedFactor{1.1};
constexpr auto levelSize{64};

#define LEV(x,y) level[((x) * levelSize) + (y)]
#define RND(min,max) ((rand() % ((max)-(min)))+(min))

struct LevelInfo
{
    int levelID;
    int textureSet;
    int textureSetSize;
    int npcCount;
    int itemCount;
    int pillarTexCount;
    int debrisTexCount;
    int npcTexCount;
    char name[32];
    char pillarTex[16][64];
    char debrisTex[16][64];
    char npcTex[8][64];
};

struct NPC
{
    double x;
    double y;
    double distance;
    int firstTexture;
    int currentTexture;
    bool alive;
    int targetX;
    int targetY;
};

using NPCs = std::vector<NPC>;

struct Item
{
    double x;
    double y;
    int nottaken;
    int number;
};

using Items = std::vector<Item>;

struct GameConfig
{
    int sWidth;
    int sHeight;
    int fullScreen;
    int level;
    int pX;
    int pY;
    int dir;
    char title[64];

    GameConfig()
    {
        FILE* cfile;

        printf("Loading configuration file... ");
        cfile = fopen("ray.cfg", "r");
        assert(cfile);
        fscanf(cfile, "%d %d %d %d %d %d %d %[^\n]", &sWidth, &sHeight, &fullScreen, &level, &pX, &pY, &dir, title);
        fclose(cfile);
        printf("OK\n");
    }
};

struct Sprite
{
    double x;
    double y;
    int texture;
};

enum class GameMode
{
    Initial,
    MainMenu,
    Game,
    GameOver,
    Quit
};
