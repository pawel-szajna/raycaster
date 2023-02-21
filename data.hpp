#pragma once

/* uncomment for release */
/*#define NDEBUG*/

#include "SDL/SDL.h"
#include <cassert>
#include <vector>

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

struct PlayerLevel
{
	int levelID;
	char visited[levelSize * levelSize];
	Items items;
	NPCs npcs;

	PlayerLevel* next;
};

struct PlayerData
{
	PlayerLevel* levels;
	PlayerLevel* current;
	
};

struct Player
{
	double posX, posY;
	double dirX, dirY;
	double planeX, planeY;

	double hpNow, hpMax; /* unused in techdemo */

	int revolver;
	int flashlight;
	int bullets;
	int battery;

	int level;
	int reloadLevel;

	PlayerData data;
};

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
