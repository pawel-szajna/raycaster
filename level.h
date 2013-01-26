#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "data.h"

int BlockType(int* level, int x, int y);
void LoadLevel(int* level, LevelInfo* li, NPCList** npcs, const char* filename);
SDL_Surface* DrawMap(int* level, Player* player);

#endif