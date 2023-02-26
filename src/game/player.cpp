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
constexpr auto at(auto x, auto y) { return x * levelSize + y; }
constexpr auto canEnter(auto x) { return x != 1 and x != 2; }
constexpr auto walkable(Level::TileArray& level, int x, int y) { return canEnter(level[at(x, y)] % 16); }
}

Level& Player::currentLevel()
{
    return current->second;
}

void Player::shoot(AI& ai)
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
            or ai.killNpc(bulletX, bulletY))
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
    double movementSpeed = frameTime * 1.6 * speedFactor; /* pola/sekunde */
    double rotationSpeed = frameTime * 1.2 * speedFactor; /* radiany/sekunde */
    int collision;

    auto& posX = position.x;
    auto& posY = position.y;
    auto& dirX = position.dirX;
    auto& dirY = position.dirY;
    auto& planeX = position.planeX;
    auto& planeY = position.planeY;

    auto& level = currentLevel().map;

    double walkFactor = 0;
    double strafeFactor = 0;
    double rotationFactor = 0;

    if (keys[SDLK_DOWN]) walkFactor -= 1;
    if (keys[SDLK_UP])   walkFactor += 1;
    if (keys[SDLK_z]) strafeFactor -= 1;
    if (keys[SDLK_x]) strafeFactor += 1;
    if (keys[SDLK_RIGHT]) rotationFactor -= 1;
    if (keys[SDLK_LEFT])  rotationFactor += 1;

    if (walkFactor != 0)
    {
        auto differenceX = movementSpeed * (walkFactor * dirX + strafeFactor * sin(atan2(dirY, dirX)));
        auto differenceY = movementSpeed * (walkFactor * dirY - strafeFactor * cos(atan2(dirY, dirX)));

        if (walkable(level, position.x + differenceX * 3, position.y)) position.x += differenceX;
        if (walkable(level, position.x, position.y + differenceY * 3)) position.y += differenceY;

        MarkVisited(this, (int)(position.x), (int)(position.y));
    }

    if (rotationFactor != 0)
    {
        auto oldDir = dirX;
        dirX = dirX * cos(rotationSpeed * rotationFactor) - dirY * sin(rotationSpeed * rotationFactor);
        dirY = oldDir * sin(rotationSpeed * rotationFactor) + dirY * cos(rotationSpeed * rotationFactor);

        oldDir = planeX;
        planeX = planeX * cos(rotationSpeed * rotationFactor) - planeY * sin(rotationSpeed * rotationFactor);
        planeY = oldDir * sin(rotationSpeed * rotationFactor) + planeY * cos(rotationSpeed * rotationFactor);
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