#ifndef _DATA_H_
#define _DATA_H_

/* uncomment for release */
/*#define NDEBUG*/

#include "SDL/SDL.h"
#include <assert.h>

#define SFACTOR 1.1
#define LEVEL_SIZE 64
#define LEV(x,y) level[((x) * LEVEL_SIZE) + (y)]
#define RND(min,max) ((rand() % ((max)-(min)))+(min))

typedef struct 
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

} LevelInfo;

typedef struct
{
	double x;
	double y;
	double dist;
	int firstTexture;
	int currentTexture;
	int alive;
	int targetX;
	int targetY;

} NPC;

typedef struct NPCList
{
	NPC npc;
	struct NPCList* next;

} NPCList;

typedef struct
{
	double x;
	double y;
	int nottaken;
	int number;

} Item;

typedef struct ItemList
{
	Item item;
	struct ItemList* next;

} ItemList;

typedef struct PlayerLevel
{
	int levelID;
	char visited[LEVEL_SIZE * LEVEL_SIZE];
	ItemList* items;
	NPCList* npcs;

	struct PlayerLevel* next;

} PlayerLevel;

typedef struct
{
	PlayerLevel* levels;
	PlayerLevel* current;
	
} PlayerData;

typedef struct
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

} Player;

typedef struct
{
	int sWidth;
	int sHeight;
	int fullScreen;
	int level;
	int pX;
	int pY;
	int dir;
	char title[64];

} GameConfig;

typedef struct
{
	double x;
	double y;
	int texture;
	
} Sprite;

enum GameModes
{
	MODE_INIT,
	MODE_MAIN_MENU,
	MODE_GAME,
	MODE_GAMEOVER,
	MODE_QUIT
};

#endif
