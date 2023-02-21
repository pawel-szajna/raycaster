#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <math.h>

#include "level.hpp"
#include "ai.hpp"

void clear_level(int* level);
void depth_first(int* level, int a, int b, int c, int d, int e, int f, int g);
void random_room(int* level);
void drawmap(int* level, int pX, int pY);
void corridor(int* level);
void generate_map(int* level, int pX, int pY, int bonusroom);
void generate_npcs(int* level, NPCList** npcs);

#endif