#ifndef _AI_H_
#define _AI_H_

#define TICK_FREQUENCY 0.2

#include "data.h"

void ResetAI(NPCList** npcs);
void InitAI(int* level);
int KillNPC(double x, double y, Player* player);
int AI_Tick(Player* player, double frameTime, int flashlight);
double AI_DistanceToNearestNPC(Player* player);
NPCList* AddNPC(NPCList* npcs, double x, double y, int firstTexture);
ItemList* AddItem(ItemList* item, double x, double y, int num);

#endif