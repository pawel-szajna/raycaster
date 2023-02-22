#pragma once

#include "data.hpp"

class Player;
namespace sdl
{
class Surface;
}

int BlockType(int* level, int x, int y);
void LoadLevel(int* level, LevelInfo* li, NPCs& npcs, const char* filename);
sdl::Surface drawMap(int* level, Player& player);
