#pragma once

#include "data.hpp"

class Player;

int BlockType(int* level, int x, int y);
void LoadLevel(int* level, LevelInfo* li, NPCs& npcs, const char* filename);
SDL_Surface* DrawMap(int* level, Player* player);
