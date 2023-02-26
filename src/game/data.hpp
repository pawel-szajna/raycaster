#pragma once

/* uncomment for release */
/*#define NDEBUG*/

#include <cassert>
#include <vector>
#include <algorithm>
#include <unordered_map>

constexpr auto speedFactor{1.1};
constexpr auto levelSize{64};

constexpr auto renderWidth{540};
constexpr auto renderHeight{300};

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
    int sprite;
    int targetX;
    int targetY;

    int texture() const
    {
        return alive ? (48 + firstTexture + 8 * currentTexture) : 88 + firstTexture;
    }
};

using NPCs = std::vector<NPC>;

struct Item
{
    double x;
    double y;
    bool taken;
    int number;
    int sprite;
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

    GameConfig();
};

enum class GameMode
{
    Initial,
    MainMenu,
    Game,
    GameOver,
    Quit
};
