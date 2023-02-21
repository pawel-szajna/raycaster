#pragma once

#include "data.hpp"

#define wwWidth 540
#define wwHeight 300

#define texWidth 120
#define texHeight 120
#define texCount 106

class Player;

namespace sdl
{
class Surface;
}

sdl::Surface InitCaster(int* level, LevelInfo* li);
void generateNoise(sdl::Surface& noise, int amount);
void generateNoiseLinear(sdl::Surface& noise, int amount);
void SpriteSort(int* order, double* distance, int count);
void CastFrame(SDL_Surface* worldview, int* worldMap, Player* player, int flashlight);
void ResetDynamicSprites();
void AddDynamicSprite(double x, double y, int texture);
int GetCasterWidth();
int GetCasterHeight();
