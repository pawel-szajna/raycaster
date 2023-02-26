#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <math.h>

#include "level.hpp"
#include "ai.hpp"

void clear_level(Level::TileArray& level);
void depth_first(Level::TileArray& level, int a, int b, int c, int d, int e, int f, int g);
void random_room(Level::TileArray& level);
void drawmap(Level::TileArray& level, int pX, int pY);
void corridor(Level::TileArray& level);
void generate_map(Level::TileArray& level, int pX, int pY, int bonusroom);
NPCs generate_npcs(Level::TileArray& level);

#endif