#include "generator.hpp"

#include <spdlog/spdlog.h>

namespace
{
constexpr auto EMPTY{0};
constexpr auto WALL{1};
constexpr auto PILLAR{2};

constexpr auto xy(int x, int y) { return levelSize * x + y; }

struct Neighbour
{
    int x;
    int y;
    int connectionX;
    int connectionY;
};
}

Generator::Generator(Level::TileArray &level) :
    level(level)
{}

int& Generator::at(int x, int y)
{
    return level[xy(x, y)];
}

bool Generator::wallAt(int x, int y)
{
    return at(x, y) % 16 == 1;
}

void Generator::fillMap(int playerX, int playerY, bool bonusRoom)
{
    spdlog::info("Generating level structure");

    clearLevel();
    dig(playerX, playerY, playerX, playerY, false);
    at(playerX, playerY) = 0;

    for (int i = 0; i < 12; ++i) spawnRandomRoom();
    for (int i = 0; i < 4; ++i) spawnCorridor();
    if (bonusRoom) spawnBonusRoom();

    fixWallTextures();
}

void Generator::clearLevel()
{
    auto empty = 1 + (1 << 4) + (1 << 8) + (1 << 12) + (1 << 16) + (1 << 20) + (1 << 24);
    std::fill(level.begin(), level.end(), empty);
}

void Generator::dig(int currentX, int currentY, int connectionX, int connectionY, bool fill)
{
    std::vector<Neighbour> neighbours{};
    neighbours.reserve(4);

    if (at(currentX, currentY) == EMPTY)
    {
        return;
    }

    if (fill)
    {
        at(currentX, currentY) = WALL;
        return;
    }

    at(currentX, currentY) = EMPTY;
    at(connectionX, connectionY) = EMPTY;

    if (currentX > 1) neighbours.push_back({currentX - 2, currentY, currentX - 1, currentY});
    if (currentY > 1) neighbours.push_back({currentX, currentY - 2, currentX, currentY - 1});
    if (currentX < levelSize - 3) neighbours.push_back({currentX + 2, currentY, currentX + 1, currentY});
    if (currentY < levelSize - 3) neighbours.push_back({currentX, currentY + 2, currentX, currentY + 1});

    std::shuffle(neighbours.begin(), neighbours.end(), rng);
    for (const auto& neighbour : neighbours)
    {
        dig(neighbour.x, neighbour.y,
            neighbour.connectionX, neighbour.connectionY,
            neighbours.size() >= 3 and ((rand() % 6) - 4) > 0);
    }
}

void Generator::fixWallTextures()
{
    constexpr static auto LIMIT{levelSize - 1};
    constexpr static auto noBricks{1};
    constexpr static auto bricksLeft{5};
    constexpr static auto bricksRight{3};
    constexpr static auto bricksBoth{4};

    for (int x = 0; x < levelSize; ++x) for(int y = 0; y < levelSize; ++y) if (at(x, y) % 16 == 1)
    {
        int south{noBricks}, east{noBricks}, north{noBricks}, west{noBricks};

        if (x != LIMIT and y != 0 and y != LIMIT)
        {
            if (wallAt(x + 1, y - 1) or not wallAt(x, y - 1))
            {
                if (wallAt(x + 1, y + 1) or not wallAt(x, y + 1)) south = bricksBoth;
                else south = bricksLeft;
            }
            else if (wallAt(x + 1, y + 1) or not wallAt(x, y + 1)) south = bricksRight;
        }

        if (x != 0 and y != 0 and y != LIMIT)
        {
            if (wallAt(x - 1, y - 1) or not wallAt(x, y - 1))
            {
                if (wallAt(x - 1, y + 1) or not wallAt(x, y + 1)) north = bricksBoth;
                else north = bricksRight;
            }
            else if (wallAt(x - 1, y + 1) or not wallAt(x, y + 1)) north = bricksLeft;
        }

        if (y != LIMIT and x != 0 and x != LIMIT)
        {
            if (wallAt(x - 1, y + 1) or not wallAt(x - 1, y))
            {
                if (wallAt(x + 1, y + 1) or not wallAt(x + 1, y)) east = bricksBoth;
                else east = bricksRight;
            }
            else if (wallAt(x + 1, y + 1) or not wallAt(x + 1, y)) east = bricksLeft;
        }

        if (y != 0 and x != 0 and x != LIMIT)
        {
            if (wallAt(x - 1, y - 1) or not wallAt(x - 1, y))
            {
                if (wallAt(x + 1, y - 1) or not wallAt(x + 1, y)) west = bricksBoth;
                else west = bricksLeft;
            }
            else if (wallAt(x + 1, y - 1) or not wallAt(x + 1, y)) west = bricksRight;
        }

        at(x, y) = 1 + (south << 4) + (east << 8) + (north << 12) + (west << 16);
    }
}

NPCs Generator::generateNpcs(AI& ai)
{
    NPCs npcs{};

    int dx, dy;
    int npcsToGenerate = levelSize / 4;

    spdlog::debug("Generating {} NPCs", npcsToGenerate);

    while (npcsToGenerate > 0)
    {
        dx = rand() % levelSize;
        dy = rand() % levelSize;
        if (at(dx, dy) % 16 == 0)
        {
            --npcsToGenerate;
            ai.addNpc(dx, dy, 0);
        }
    }

    return npcs;
}

void Generator::spawnBonusRoom()
{
    for (int x = 43; x <= 50; ++x) at(27, x) = EMPTY;
    for (int x = 46; x <= 48; ++x) at(26, x) = at(28, x) = PILLAR;
    for (int x = 45; x <= 49; ++x) at(25, x) = at(29, x) = WALL;

    at(26, 45) = at(28, 45) = at(26, 49) = at(28, 49) = at(26, 42) = at(26, 43)
               = at(26, 44) = at(28, 42) = at(28, 43) = at(28, 44) = at(27, 42)
               = WALL;
}

void Generator::spawnRandomRoom()
{
    int width = (rand() % 2) + 4;
    int height = (rand() % 2) + 4;
    int roomX = (rand() % (levelSize - width - 3)) + 1;
    int roomY = (rand() % (levelSize - height - 3)) + 1;

    width += roomX;
    height += roomY;

    for(int x = roomX; x < width; ++x)
    {
        for(int y = roomY; y < height; ++y)
        {
            at(x, y) = EMPTY;
        }
    }
}

void Generator::spawnCorridor()
{
    int width = RND(5, 7);
    int length = RND(4, levelSize / 5);
    int startX = RND(3, levelSize - length - 3);
    int startY = RND(2, levelSize - 6);
    int endX = startX + length;
    int endY = startY + width;

    bool rotate = rand() % 2;
    if (rotate)
    {
        std::swap(startX, startY);
        std::swap(endX, endY);
    }

    for (int x = startX; x < endX; ++x)
    {
        for (int y = startY; y < endY; ++y)
        {
            at(x, y) = EMPTY;
        }
    }

    for (int x = startX + 1; x < endX; x += 2) // Big pillars
    {
        at(x, startY + 1) = WALL;
        at(x, endY - 2) = WALL;
    }
}
