#pragma once

#include "data.hpp"

void NewGame(Player* player, GameConfig* config);
void LoadConfig(GameConfig* config);
void LoadText(char* texts);

void HandleMovement(Player* player, Uint8* keys, int* level, char* visited, double frameTime);
void Shoot(int* level, Player* player);

int OnKeyPress(SDL_Event* event, int key);

int Blink(int battery);
