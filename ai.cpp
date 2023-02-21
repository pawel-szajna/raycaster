#include "ai.hpp"

#include "caster.hpp"
#include "player.hpp"

#include <cstdio>
#include <cmath>
#include <algorithm>

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

int AIMap[levelSize][levelSize];
int AISearchMap[levelSize][levelSize];
int goOn;

void InitAI(int* level)
{
    int x, y;

    for(x = 0; x < levelSize - 1; ++x)
    {
        AIMap[x][0] = 0;
        AIMap[x][levelSize - 1] = 0;
        AIMap[0][x] = 0;
        AIMap[levelSize - 1][x] = 0;
    }

    for(x = 1; x < levelSize; ++x) for(y = 1; y < levelSize; ++y)
    {
    /*	AIMap[x][y] = 0;
        if(CANENTER(levelId[32*(x+1)+y]%16)) AIMap[x][y] += 1;
        if(CANENTER(levelId[32*x+y+1]%16)) AIMap[x][y] += 2;
        if(CANENTER(levelId[32*(x-1)+y]%16)) AIMap[x][y] += 4;
        if(CANENTER(levelId[32*x+y-1]%16)) AIMap[x][y] += 8;	*/
        AIMap[x][y] = CANENTER(level[levelSize * x + y] % 16);
    }

/*test MIN2 i MIN4*/
/*printf("1 2 : %d\n7 3 : %d\n1 3 7 4 : %d\n8 3 5 5 : %d\n9 4 1 7 : %d\n8 5 9 0 : %d\n\n", MIN2(1,2), MIN2(7,3), MIN4(1,3,7,4), MIN4(8,3,5,5), MIN4(9,4,1,7),MIN4(8,5,9,0));*/
    /*for(x = 0; x < levelSize; ++x) { for(y = 0; y < levelSize; ++y) printf(AIMap[x][y] ? " " : "#"); printf("\n"); }*/
}

void ResetAI(NPCs& npcs)
{
    printf("Initializing artificial intelligence... ");
    npcs.clear();
}

void AddNPC(NPCs& npcs, double x, double y, int firstTexture)
{
    printf("Adding new NPC... ");
    npcs.push_back(NPC{x + 0.5, y + 0.5, 0, firstTexture, 0, 1, 0, 0});
}

void AddItem(Items& items, double x, double y, int num)
{
    printf("Adding new item... ");
    items.push_back(Item{x + 0.5, y + 0.5, 1, num});
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
    for(int x = 0; x < levelSize; ++x)
    {
        for(int y = 0; y < levelSize; ++y) printf("%03d ", AISearchMap[x][y]);
        printf("\n");
    }
}

void UpdateSearchMap(int cx, int cy, int tx, int ty)
{
    int x, y;

    for(x = 0; x < levelSize; ++x) for(y = 0; y < levelSize; ++y)
        AISearchMap[x][y] = 100;

    goOn = 1;
    UpdateNode(cx, cy, tx, ty, 0);
    /*PrintSearchMap();*/
}

double AI_DistanceToNearestNPC(Player* player)
{
    auto nearest = 100.0;

    for (auto& npc : player->currentLevel().npcs)
    {
        if (npc.alive)
        {
            auto distance = DIST(npc.x, npc.y, player->getPosition().x, player->getPosition().y);
            nearest = std::min(distance, nearest);
            npc.distance = distance;
        }
    }

    return sqrt(nearest);
}

bool KillNPC(double x, double y, NPCs& npcs)
{
    auto npcToKill = std::find_if(npcs.begin(),
                                  npcs.end(),
                                  [x, y] (const auto& npc)
                                  {
                                      return npc.alive and
                                             abs(npc.x - x) < 0.4 and
                                             abs(npc.y - y) < 0.4;
                                  });

    if (npcToKill == npcs.end())
    {
        return false;
    }

    npcToKill->alive = false;
    return true;
}

int AI_Tick(Player* player, double frameTime, int flashlight)
{
    double distance, target;
    int popup{};

    lastTick += frameTime;
    if(lastTick < TICK_FREQUENCY)
    {
        return popup;
    }

    ResetDynamicSprites();

    auto& currentLevel = player->currentLevel();
    const auto& position = player->getPosition();

    for (auto& npc : currentLevel.npcs)
    {
        if (npc.alive && npc.distance < SQR(8))
        {
            npc.currentTexture = (npc.currentTexture + 1) % 4;

            if (distance < 0.12 or distance > 1)
            {
                npc.targetX = (int)position.x;
                npc.targetY = (int)position.y;
                distance = sqrt(DIST(npc.x, npc.y, (double)npc.targetX + 0.5, (double)npc.targetY + 0.5));
            }

            target = (((double)npc.targetX - npc.x + 0.5) / distance) * 0.15;
            if(AIMap[(int)(npc.x + target)][(int)(npc.y)])
            {
                npc.x += target;
            }

            target = (((double)npc.targetY - npc.y + 0.5) / distance) * 0.15;
            if(AIMap[(int)(npc.x)][(int)(npc.y + target)])
            {
                npc.y += target;
            }
        }

        auto npcTexture = npc.alive ? (48 + npc.firstTexture + 8 * npc.currentTexture) : 88 + npc.firstTexture;
        AddDynamicSprite(npc.x, npc.y, npcTexture);
    }

    for (auto& item : currentLevel.items)
    {
        if ((int)position.x == (int)item.x and
            (int)position.y == (int)item.y and
            item.nottaken)
        {
            switch (item.number)
            {
            case 0:
                player->revolver = 1;
                break;
            case 1:
                player->flashlight = 1;
                break;
            case 2:
                player->bullets += 5;
                break;
            case 3:
                player->battery += 50;
                break;
            default:
                assert(!("Unknown item ID!"));
            }

            popup = item.number + 5;
            item.nottaken = 0;
        }

        if (item.nottaken)
        {
            AddDynamicSprite(item.x, item.y, 96 + item.number);
        }
    }

    if (player->battery and flashlight)
    {
        player->battery--;
    }

    lastTick = 0;
    return popup;
}
