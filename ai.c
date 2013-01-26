#include "ai.h"
#include "caster.h"

#include <stdio.h>
#include <math.h>

#define CANENTER(x) 	(!((x)==1 || (x)==2))
#define SQR(x) 			((x)*(x))
#define DIST(a,b,c,d) 	(SQR((a)-(c))+SQR((b)-(d)))

#define M_DOWN 	1
#define M_RIGHT 2
#define M_UP 	3
#define M_LEFT 	4

#define MIN2(x,y) (((x)<(y))?(x):(y))
#define MIN3(x,y,z) ((((x)<(y))&&((x)<(z)))?(x):MIN2((y),(z)))
#define MIN4(a,b,c,d) (MIN2(MIN2((a),(b)),MIN2((c),(d))))

#define MAX2(x,y) (MIN2(-(x),-(y)))
#define MAX3(x,y,z) (MIN3(-(x),-(y),-(z)))
#define MAX4(a,b,c,d) (MIN4(-(a),-(b),-(c),-(d)))

double lastTick;

int AIMap[LEVEL_SIZE][LEVEL_SIZE];
int AISearchMap[LEVEL_SIZE][LEVEL_SIZE];
int goOn;

void InitAI(int* level)
{
	int x, y;

	for(x = 0; x < LEVEL_SIZE - 1; ++x)
	{
		AIMap[x][0] = 0;
		AIMap[x][LEVEL_SIZE - 1] = 0;
		AIMap[0][x] = 0;
		AIMap[LEVEL_SIZE - 1][x] = 0;
	}

	for(x = 1; x < LEVEL_SIZE; ++x) for(y = 1; y < LEVEL_SIZE; ++y)
	{
	/*	AIMap[x][y] = 0;
		if(CANENTER(level[32*(x+1)+y]%16)) AIMap[x][y] += 1;
		if(CANENTER(level[32*x+y+1]%16)) AIMap[x][y] += 2;
		if(CANENTER(level[32*(x-1)+y]%16)) AIMap[x][y] += 4;
		if(CANENTER(level[32*x+y-1]%16)) AIMap[x][y] += 8;	*/
		AIMap[x][y] = CANENTER(level[LEVEL_SIZE*x+y]%16);
	}

/*test MIN2 i MIN4*/
/*printf("1 2 : %d\n7 3 : %d\n1 3 7 4 : %d\n8 3 5 5 : %d\n9 4 1 7 : %d\n8 5 9 0 : %d\n\n", MIN2(1,2), MIN2(7,3), MIN4(1,3,7,4), MIN4(8,3,5,5), MIN4(9,4,1,7),MIN4(8,5,9,0));*/
	/*for(x = 0; x < LEVEL_SIZE; ++x) { for(y = 0; y < LEVEL_SIZE; ++y) printf(AIMap[x][y] ? " " : "#"); printf("\n"); }*/
}

void ResetAI(NPCList** npcs)
{
	NPCList* tmp;

	printf("Initializing artificial intelligence... ");

	while(*npcs != NULL)
	{
		tmp = *npcs;
		*npcs = (*npcs)->next;
		free(tmp);
	}

	printf("OK\n");
}

NPCList* AddNPC(NPCList* npcs, double x, double y, int firstTexture)
{
	NPCList* newnpc = malloc(sizeof(NPCList));

	assert(newnpc);
	printf("Adding new NPC... ");
	newnpc->npc.x = x + 0.5;
	newnpc->npc.y = y + 0.5;
	newnpc->npc.firstTexture = firstTexture;
	newnpc->npc.currentTexture = 0;
	newnpc->npc.alive = 1;
	newnpc->next = npcs;
	printf("OK\n");

	return newnpc;
}

ItemList* AddItem(ItemList* item, double x, double y, int num)
{
	ItemList* newitems = malloc(sizeof(ItemList));

	assert(newitems);
	printf("Adding new item... ");
	newitems->item.x = x + 0.5;
	newitems->item.y = y + 0.5;
	newitems->item.number = num;
	newitems->item.nottaken = 1;
	newitems->next = item;
	printf("OK\n");

	return newitems;
}

void UpdateNode(int cx, int cy, int tx, int ty, int value)
{
	int newv;

	if(!goOn) return;
	if(!AIMap[cx][cy]) return;

	newv = value + abs(cx - tx) + abs(cy - ty);
	if(newv > 100) return;

	if(AISearchMap[cx][cy] > newv)
	{
		AISearchMap[cx][cy] = newv;
		if(cx == tx && cy == ty)
		{
			goOn = 0;
			return;
		}
		UpdateNode(cx - 1, cy, tx, ty, value + 1);
		UpdateNode(cx + 1, cy, tx, ty, value + 1);
		UpdateNode(cx, cy - 1, tx, ty, value + 1);
		UpdateNode(cx, cy + 1, tx, ty, value + 1);
	}
}

void PrintSearchMap()
{
	for(int x = 0; x < LEVEL_SIZE; ++x)
	{
		for(int y = 0; y < LEVEL_SIZE; ++y) printf("%03d ", AISearchMap[x][y]);
		printf("\n");
	}
}

void UpdateSearchMap(int cx, int cy, int tx, int ty)
{
	int x, y;

	for(x = 0; x < LEVEL_SIZE; ++x) for(y = 0; y < LEVEL_SIZE; ++y)
		AISearchMap[x][y] = 100;

	goOn = 1;
	UpdateNode(cx, cy, tx, ty, 0);
	/*PrintSearchMap();*/
}

double AI_DistanceToNearestNPC(Player* player)
{
	NPCList* current;
	double distance, nearest;
	nearest = 100;

	current = player->data.current->npcs;
	assert(current);

	while(current != NULL)
	{
		if(current->npc.alive)
		{
			distance = DIST(current->npc.x, current->npc.y, player->posX, player->posY);
			nearest = distance < nearest ? distance : nearest;
			current->npc.dist = distance;
		}
		current = current->next;
	}

	return sqrt(nearest);
}

int KillNPC(double x, double y, Player* player)
{
	NPCList* current = player->data.current->npcs;
	while(current != NULL)
	{
		if(current->npc.alive && (abs(current->npc.x - x) < 0.4) && (abs(current->npc.y - y) < 0.4))
		{
			current->npc.alive = 0;
			return 1;
		}
		current = current->next;
	}
	return 0;
}

int AI_Tick(Player* player, double frameTime, int flashlight)
{
	NPCList* current;
	ItemList* currentitems;
	double distance, target;
	int popup=0;

	lastTick += frameTime;
	if(lastTick < TICK_FREQUENCY) return popup;

	ResetDynamicSprites();

	current = player->data.current->npcs;
	currentitems = player->data.current->items;
	assert(current);

	while(current != NULL) /* handle npcs */
	{
		if(current->npc.alive && (current->npc.dist < SQR(8)))
		{
			current->npc.currentTexture = (current->npc.currentTexture + 1) % 4;
			distance = sqrt(DIST(current->npc.x, current->npc.y, (double)current->npc.targetX + 0.5, (double)current->npc.targetY + 0.5));

			/* poki co kazdy npc szuka gracza */
			if(distance < 0.12 || distance > 1)
			{
				/* wyznaczanie nowego celu */
				current->npc.targetX = (int)player->posX;
				current->npc.targetY = (int)player->posY;
				distance = sqrt(DIST(current->npc.x, current->npc.y, (double)current->npc.targetX + 0.5, (double)current->npc.targetY + 0.5));
				/*UpdateSearchMap((int)current->npc.x, (int)current->npc.y, (int)player->posX, (int)player->posY);*/
			}

			target = (((double)current->npc.targetX - current->npc.x + 0.5) / distance) * 0.15;
			if(AIMap[(int)(current->npc.x + target)][(int)(current->npc.y)]) current->npc.x += target;

			target = (((double)current->npc.targetY - current->npc.y + 0.5) / distance) * 0.15;
			if(AIMap[(int)(current->npc.x)][(int)(current->npc.y + target)]) current->npc.y += target;
		}

		AddDynamicSprite(current->npc.x, current->npc.y, current->npc.alive ? (48 + current->npc.firstTexture + 8 * current->npc.currentTexture) : 88 + current->npc.firstTexture);
		current = current->next;
	}

	while(currentitems != NULL) /* handle items */
	{	
		if((int)player->posX == (int)currentitems->item.x && (int)player->posY == (int)currentitems->item.y && currentitems->item.nottaken)
		{
			switch(currentitems->item.number)
			{
				case 0: player->revolver = 1; break;
				case 1: player->flashlight = 1; break;
				case 2: player->bullets += 5; break;
				case 3: player->battery += 50; break;
				default: assert(!("Unknown item ID!"));
			}
			popup = currentitems->item.number+5;
			currentitems->item.nottaken = 0;
		}

		if(currentitems->item.nottaken) AddDynamicSprite(currentitems->item.x, currentitems->item.y, 96 + currentitems->item.number);
		currentitems = currentitems->next;
	}

	if(player->battery > 0 && flashlight) player->battery--;

	lastTick = 0;

	return popup;
}
