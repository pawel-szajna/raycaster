#include "player.hpp"

#include "data.hpp"
#include "level.hpp"
#include "ai.hpp"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <spdlog/spdlog.h>

#include "SDL/SDL.h"

namespace
{
constexpr auto canEnter(auto x) { return x != 1 and x != 2; }
}

Level& Player::currentLevel()
{
    return current->second;
}

void Player::shoot()
{
    if (not revolver)
    {
        return;
    }

    double bulletX = position.x;
    double bulletY = position.y;

    auto& level = currentLevel().map;

    for (int i = 0; i < 100; ++i)
    {
        if (not canEnter(level[(int)bulletX * levelSize + (int)bulletY] % 16)
            or bulletX < 0 or bulletY < 0
            or bulletX > levelSize
            or bulletY > levelSize
            or KillNPC(bulletX, bulletY, currentLevel().npcs))
        {
            break;
        }

        bulletX += position.dirX * 2;
        bulletY += position.dirY * 2;
    }
}

void LoadText(char* texts)
{
    FILE* txt;
    char text[128];

    spdlog::info("Loading texts...");
    txt = fopen("txt/eng.txt", "r");
    assert(txt);
    for(int i = 0; i < 6; ++i)
    {
        fgets(text, sizeof(text), txt); /* fscanf nie przechodzil do nowej linii, trzeba tak ;_; */
        strcpy(&texts[128 * i], text); /* bier go */
        texts[128 * i + strlen(text) - 1] = '\0'; /* debil zostawia znak nowej linii w stringu */
    }
    fclose(txt);
}

Player::Player(const GameConfig& config) :
    levelId(config.level),
    position{.x = (double)config.pX + 0.5,
             .y = (double)config.pY + 0.5,
             .dirX = -cos(config.dir * 0.7854),
             .dirY = -sin(config.dir * 0.7854),
             .planeX = -0.77 * sin(config.dir * 0.7854),
             .planeY =  0.77 * cos(config.dir * 0.7854)}

{}

void MarkVisitedSub(std::array<bool, 4096>& visited, int x, int y)
{
    if (x < 0 or  y < 0 or x >= levelSize or y >= levelSize)
    {
        return;
    }

    visited[levelSize * x + y] = 1;
}

void MarkVisited(Player* player, int x, int y)
{
    auto& visited = player->currentLevel().visited;

    if (visited[levelSize * x + y])
    {
        return;
    }

    visited[levelSize * x + y] = 2;

    MarkVisitedSub(visited, x - 1, y - 1);
    MarkVisitedSub(visited, x - 1, y);
    MarkVisitedSub(visited, x - 1, y + 1);
    MarkVisitedSub(visited, x, y + 1);
    MarkVisitedSub(visited, x, y - 1);
    MarkVisitedSub(visited, x + 1, y + 1);
    MarkVisitedSub(visited, x + 1, y);
    MarkVisitedSub(visited, x + 1, y - 1);
    MarkVisitedSub(visited, x, y - 2);
    MarkVisitedSub(visited, x, y + 2);
    MarkVisitedSub(visited, x - 2, y);
    MarkVisitedSub(visited, x + 2, y);
}

void Player::switchLevel()
{
    if (not levels.contains(levelId))
    {
        levels.emplace(levelId, Level(std::format("map/level{}.dat", levelId)));
    }

    current = levels.find(levelId);
}

void Player::handleMovement(uint8_t* keys, double frameTime)
{
    double mSpeed = frameTime * 1.6 * speedFactor; /* pola/sekunde */
    double rSpeed = frameTime * 1.2 * speedFactor; /* radiany/sekunde */
    double oldDir;
    int collision;

    auto& posX = position.x;
    auto& posY = position.y;
    auto& dirX = position.dirX;
    auto& dirY = position.dirY;
    auto& planeX = position.planeX;
    auto& planeY = position.planeY;

    auto& level = currentLevel().map;

    if (keys[SDLK_UP])
    {
        collision = level[(int)(posX + dirX * mSpeed * 3) * levelSize + (int)posY] % 16;
        if (canEnter(collision)) posX += dirX * mSpeed;
        collision = level[(int)posX * levelSize + (int)(posY + dirY * mSpeed * 3)] % 16;
        if (canEnter(collision)) posY += dirY * mSpeed;
        MarkVisited(this, (int)(posX), (int)(posY));
    }

    if (keys[SDLK_DOWN])
    {
        collision = level[(int)(posX - dirX * mSpeed * 3) * levelSize + (int)posY] % 16;
        if (canEnter(collision)) posX -= dirX * mSpeed;
        collision = level[(int)posX * levelSize + (int)(posY - dirY * mSpeed * 3)] % 16;
        if (canEnter(collision)) posY -= dirY * mSpeed;
        MarkVisited(this, (int)(posX), (int)(posY));
    }

    if (keys[SDLK_z])
    {
        collision = level[(int)(posX - 3 * mSpeed * sin(atan2(dirY, dirX))) * levelSize + (int)posY] % 16;
        if (canEnter(collision)) posX -= mSpeed * sin(atan2(dirY, dirX));
        collision = level[(int)posX * levelSize + (int)(posY + 3 * mSpeed * cos(atan2(dirY, dirX)))] % 16;
        if (canEnter(collision)) posY += mSpeed * cos(atan2(dirY, dirX));
        MarkVisited(this, (int)(posX), (int)(posY));
    }

    if (keys[SDLK_x])
    {
        collision = level[(int)(posX + 3 * mSpeed * sin(atan2(dirY, dirX))) * levelSize + (int)posY] % 16;
        if (canEnter(collision)) posX += mSpeed * sin(atan2(dirY, dirX));
        collision = level[(int)posX * levelSize + (int)(posY - 3 * mSpeed * cos(atan2(dirY, dirX)))] % 16;
        if (canEnter(collision)) posY -= mSpeed * cos(atan2(dirY, dirX));
        MarkVisited(this, (int)(posX), (int)(posY));
    }

    if (keys[SDLK_LEFT])
    {
        oldDir = dirX;
        dirX = dirX * cos(rSpeed) - dirY * sin(rSpeed);
        dirY = oldDir * sin(rSpeed) + dirY * cos(rSpeed);

        oldDir = planeX;
        planeX = planeX * cos(rSpeed) - planeY * sin(rSpeed);
        planeY = oldDir * sin(rSpeed) + planeY * cos(rSpeed);
    }

    if (keys[SDLK_RIGHT])
    {
        oldDir = dirX;
        dirX = dirX * cos(-rSpeed) - dirY * sin(-rSpeed);
        dirY = oldDir * sin(-rSpeed) + dirY * cos(-rSpeed);

        oldDir = planeX;
        planeX = planeX * cos(-rSpeed) - planeY * sin(-rSpeed);
        planeY = oldDir * sin(-rSpeed) + planeY * cos(-rSpeed);
    }

//    if(level[(int)posX * levelSize + (int)posY] % 16 == 6)
//    {
//        a = level[(int)posX * levelSize + (int)posY];
//
//        levelId = (a >> 8) % 16;
//        posX = ((a >> 12) % 256) + 0.5;
//        posY = ((a >> 20) % 256) + 0.5;
//        reloadLevel = true;
//    }
}

int Player::blink()
{
    if (battery <= 0 or not flashlight)
    {
        return 0;
    }

    if (battery > 45)
    {
        return 1;
    }

    if (battery < 10)
    {
        return !(rand() % (10 - battery));
    }

    return (rand() % battery);
}