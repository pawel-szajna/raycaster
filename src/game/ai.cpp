#include "ai.hpp"

#include "raycaster/caster.hpp"
#include "player.hpp"

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <spdlog/spdlog.h>

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

AI::AI(Player &player,
       std::unique_ptr<raycaster::Caster>& caster) :
    player(player),
    caster(caster)
{
}

void AI::registerSprites()
{
    if (caster == nullptr)
    {
        spdlog::error("The raycaster must not be nullptr now");
        return;
    }

    auto& level = player.currentLevel();

    for (auto& item : level.items)
    {
        if (not item.taken)
        {
            item.sprite = caster->addSprite(item.x, item.y, item.number + 96);
        }
    }

    for (auto& npc : level.npcs)
    {
        npc.sprite = caster->addSprite(npc.x, npc.y, npc.texture());
    }
}

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

void UpdateSearchMap(int cx, int cy, int tx, int ty)
{
    int x, y;

    for(x = 0; x < levelSize; ++x) for(y = 0; y < levelSize; ++y)
        AISearchMap[x][y] = 100;

    goOn = 1;
    UpdateNode(cx, cy, tx, ty, 0);
    /*PrintSearchMap();*/
}

double AI::distanceToNearestNpc()
{
    auto nearest = 100.0;

    for (auto& npc : player.currentLevel().npcs)
    {
        if (npc.alive)
        {
            auto distance = DIST(npc.x, npc.y, player.getPosition().x, player.getPosition().y);
            nearest = std::min(distance, nearest);
            npc.distance = distance;
        }
    }

    return sqrt(nearest);
}

bool AI::killNpc(double x, double y)
{
    auto& npcs = player.currentLevel().npcs;

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

int AI::tick(double frameTime, bool flashlight)
{
    double distance, target;
    int popup{};

    lastTick += frameTime;
    if (lastTick < TICK_FREQUENCY)
    {
        return 0;
    }

    // ResetDynamicSprites(); TODO

    auto& currentLevel = player.currentLevel();
    const auto& position = player.getPosition();

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

        if (npc.sprite > 0)
        {
            caster->updateSprite(npc.sprite, npc.x, npc.y, npc.texture());
        }
    }

    for (auto& item : currentLevel.items)
    {
        if ((int)position.x == (int)item.x and
            (int)position.y == (int)item.y and
            not item.taken)
        {
            switch (item.number)
            {
            case 0:
                player.revolver = 1;
                break;
            case 1:
                player.flashlight = 1;
                break;
            case 2:
                player.bullets += 5;
                break;
            case 3:
                player.battery += 50;
                break;
            default:
                spdlog::warn("Found unsupported item: {}", item.number);
            }

            popup = item.number + 5;
            item.taken = true;
            if (item.sprite > 0)
            {
                caster->removeSprite(item.sprite);
            }
        }

        if (not item.taken)
        {
            // AddDynamicSprite(item.x, item.y, 96 + item.number); TODO
        }
    }

    if (player.battery and flashlight)
    {
        // player->battery--;
    }

    lastTick = 0;
    return popup;
}
