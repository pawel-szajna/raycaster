#pragma once

#define TICK_FREQUENCY 0.2

#include "data.hpp"

class Player;

void ResetAI(NPCs& npcs);
void InitAI(int* level);
bool KillNPC(double x, double y, NPCs& npcs);
int AI_Tick(Player* player, double frameTime, int flashlight);
double AI_DistanceToNearestNPC(Player* player);
void AddNPC(NPCs& npcs, double x, double y, int firstTexture);
void AddItem(Items& items, double x, double y, int num);
