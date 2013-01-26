#ifndef _CASTER_H_
#define _CASTER_H_

#include "data.h"

#define wwWidth 540
#define wwHeight 300

#define texWidth 120
#define texHeight 120
#define texCount 106

SDL_Surface* InitCaster(int* level, LevelInfo* li);
void GenerateNoise(SDL_Surface* noise, int amount);
void GenerateNoiseLinear(SDL_Surface* noise, int amount);
void SpriteSort(int* order, double* distance, int count);
void CastFrame(SDL_Surface* worldview, int* worldMap, Player* player, int flashlight);
void ResetDynamicSprites();
void AddDynamicSprite(double x, double y, int texture);
int GetCasterWidth();
int GetCasterHeight();

#endif