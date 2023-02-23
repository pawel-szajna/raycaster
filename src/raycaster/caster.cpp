#include "caster.hpp"

#include "game/player.hpp"
#include "game/level.hpp"
#include "sdlwrapper/sdl.hpp"

#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace raycaster
{
Uint32 color;

struct Sprite
{
    double x;
    double y;
    int texture;
    double distance;
    bool dynamic{false}; // TODO: remove, as this is probably a big performance hit
};

std::vector<Sprite> sprites;

namespace
{
constexpr auto DYNAMIC{true};

enum class Hit : bool
{
    Horizontal,
    Vertical
};

constexpr auto sqr(auto x)
{
    return x * x;
}

constexpr auto outOfBounds(int x, int y, int size)
{
    return x < 0 or y < 0 or x >= size or y >= size;
}
}

void Caster::loadTexture(int id, const std::string& filename)
{
    spdlog::debug("Loading raycaster texture {}", filename);
    FILE *data = fopen(filename.c_str(), "rb");
    if (not data)
    {
        spdlog::warn("File not found: {}!", filename);
    }

    unsigned char dataR, dataG, dataB;
    auto& texture = textures[id];
    for (auto pixel = 0; pixel < TEXTURE_WIDTH * TEXTURE_HEIGHT; ++pixel)
    {
        fread(&dataR, sizeof(char), 1, data);
        fread(&dataG, sizeof(char), 1, data);
        fread(&dataB, sizeof(char), 1, data);
        texture[pixel] = 65536 * dataR + 256 * dataG + dataB;
    }

    fclose(data);
}

Caster::Caster(int *level, const LevelInfo& li) :
    worldView(sdl::make_surface(renderWidth, renderHeight))
{
    char filename[128];

    spdlog::info("Initializing caster");
    assert(level);

    auto numSprites = 0;
    for (int x = 0; x < levelSize; ++x)
        for (int y = 0; y < levelSize; ++y)
            if (BlockType(level, x, y) == 2 || BlockType(level, x, y) == 5 || BlockType(level, x, y) == 6)
                ++numSprites;

    sprites.clear();
    sprites.reserve(numSprites);

    for (int x = 0; x < levelSize; ++x)
        for (int y = 0; y < levelSize; ++y)
        {
            int u = BlockType(level, x, y);
            if (u == 2 || u == 5 || u == 6)
            {
                auto texture = ((level[levelSize * x + y] / 16) % 16) + (u == 2 ? 16 : (u == 5 ? 32 : 0));
                sprites.push_back(Sprite{0.5 + x, 0.5 + y, texture, 0});
            }
        }

    spdlog::info("Loading textures");

    for (int x = 1; x <= (li.textureSetSize); ++x)
    {
        sprintf(filename, "gfx/set_%d/%d.raw", li.textureSet, x);
        loadTexture(x, filename);
    }

    for (int x = 0; x < li.pillarTexCount; ++x)
    {
        sprintf(filename, "gfx/%s.raw", li.pillarTex[x]);
        loadTexture(16 + x, filename);
    }

    for (int x = 0; x < li.debrisTexCount; ++x)
    {
        sprintf(filename, "gfx/%s.raw", li.debrisTex[x]);
        loadTexture(32 + x, filename);
    }

    for (int x = 0; x < li.npcTexCount; ++x)
    {
        sprintf(filename, "gfx/%s_walk1.raw", li.npcTex[x]);
        loadTexture(48 + x, filename);
        sprintf(filename, "gfx/%s_walk2.raw", li.npcTex[x]);
        loadTexture(56 + x, filename);
        sprintf(filename, "gfx/%s_walk3.raw", li.npcTex[x]);
        loadTexture(64 + x, filename);
        sprintf(filename, "gfx/%s_walk4.raw", li.npcTex[x]);
        loadTexture(72 + x, filename);
        sprintf(filename, "gfx/%s_shoot.raw", li.npcTex[x]);
        loadTexture(80 + x, filename);
        sprintf(filename, "gfx/%s_dead.raw", li.npcTex[x]);
        loadTexture(88 + x, filename);
    }

    for (int x = 0; x < 4; ++x)
    {
        sprintf(filename, "gfx/items/%d.raw", x);
        loadTexture(96 + x, filename);
    }

    loadTexture(0, "gfx/transparent.raw");
    loadTexture(10, "gfx/portal.raw");
}

void Caster::resetDynamicSprites()
{
    spdlog::debug("Reset dynamic sprites");
    sprites.erase(std::remove_if(sprites.begin(), sprites.end(), [](const auto &sprite) { return sprite.dynamic; }),
                  sprites.end());
}

void Caster::addDynamicSprite(double x, double y, int texture)
{
    spdlog::debug("Add dynamic sprite @ {};{}", x, y);
    sprites.push_back(Sprite{x, y, texture, 0, DYNAMIC});
}

void Caster::frame(const int* worldMap, const Player& player, int flashlight)
{
    int x, y;

    auto fStart = flashlight ? 2.5 : 0.5;
    auto fEnd = flashlight ? 5.5 : 3.1;
    auto fLength = fEnd - fStart;

    double ZBuffer[renderWidth];
    Uint32 buffer[renderWidth][renderHeight];

    const auto &position = player.getPosition();

    for (x = 0; x < renderWidth; ++x)
    {
        auto camX = 2 * x / (double) renderWidth - 1; /* wspolrzedna x na plaszczyznie na ktorej rzutowany jest obraz */
        auto rayPX = position.x;
        auto rayPY = position.y;
        auto rayDX = position.dirX + position.planeX * camX;
        auto rayDY = position.dirY + position.planeY * camX;
        auto mapX = (int) rayPX;
        auto mapY = (int) rayPY;
        auto deltadX = sqrt(1 + sqr(rayDY) / sqr(rayDX));
        auto deltadY = sqrt(1 + sqr(rayDX) / sqr(rayDY));
        auto stepX = (rayDX < 0) ? -1 : 1;
        auto stepY = (rayDY < 0) ? -1 : 1;
        auto sidedX = (stepX < 0) ? (rayPX - mapX) * deltadX : (mapX + 1.0 - rayPX) * deltadX;
        auto sidedY = (stepY < 0) ? (rayPY - mapY) * deltadY : (mapY + 1.0 - rayPY) * deltadY;
        Hit side{};

        while (not outOfBounds(mapX, mapY, levelSize))
        {
            if (sidedX < sidedY)
            {
                sidedX += deltadX;
                mapX += stepX;
                side = Hit::Horizontal;
            }
            else
            {
                sidedY += deltadY;
                mapY += stepY;
                side = Hit::Vertical;
            }

            if ((worldMap[mapX * levelSize + mapY] % 16) == 1)
            {
                break;
            }
        }

        if (outOfBounds(mapX, mapY, levelSize))
        {
            continue;
        }

        auto wallDist = side == Hit::Vertical
                        ? fabs((mapY - rayPY + (1 - stepY) / 2) / rayDY)
                        : fabs((mapX - rayPX + (1 - stepX) / 2) / rayDX);
        auto lineHeight = abs((int) (renderHeight / wallDist));
        auto dStart = std::max(-lineHeight / 2 + renderHeight / 2, 0);
        auto dEnd = std::min(lineHeight / 2 + renderHeight / 2, renderHeight - 1);

        auto tex = (worldMap[mapX * levelSize + mapY] >> 4);
        if (side == Hit::Horizontal and position.x < mapX)
        {
            tex >>= 8;
        }
        if (side == Hit::Vertical)
        {
            if (position.y < mapY) tex >>= 12;
            else tex >>= 4;
        }
        tex %= 16;

        auto wallX = side == Hit::Vertical
                     ? rayPX + ((mapY - rayPY + (1 - stepY) / 2) / rayDY) * rayDX
                     : rayPY + ((mapX - rayPX + (1 - stepX) / 2) / rayDX) * rayDY;
        wallX -= floor(wallX);
        auto texX = (int) (wallX * (double) TEXTURE_WIDTH);
        if ((side == Hit::Horizontal and rayDX > 0) or (side == Hit::Vertical && rayDY < 0))
        {
            texX = TEXTURE_WIDTH - texX - 1;
        }

        for (y = dStart; y < dEnd; ++y)
        {
            auto d = y * 256 - renderHeight * 128 + lineHeight * 128;
            auto texY = ((d * TEXTURE_HEIGHT) / lineHeight) / 256;
            color = textures[tex][TEXTURE_HEIGHT * texY + texX];
            if (wallDist > fStart && wallDist < fEnd)
            {
                auto r = (color >> 16) % 256;
                auto g = (color >> 8) % 256;
                auto b = color % 256;
                r *= (1 - ((wallDist - fStart) / (fLength - 0.1)));
                g *= (1 - ((wallDist - fStart) / (fLength)));
                b *= (1 - ((wallDist - fStart) / (fLength - 0.05)));
                if (r > 255) r = 0;
                if (b > 255) b = 0;
                color = 65536 * r + 256 * g + b;
            }
            if (wallDist < fEnd) buffer[x][y] = color;
        }

        ZBuffer[x] = wallDist;
    }

    std::transform(sprites.begin(), sprites.end(), sprites.begin(),
                   [&position](auto sprite) {
                       sprite.distance = sqr(position.x - sprite.x) + sqr(position.y - sprite.y);
                       return sprite;
                   });

    std::sort(sprites.begin(), sprites.end(),
              [](const auto &a, const auto &b) { return a.distance < b.distance; });

    for (const auto &sprite: sprites)
    {
        auto invDet = 1.0 / (position.planeX * position.dirY - position.dirX * position.planeY);

        auto transX = invDet * (position.dirY * (sprite.x - position.x) - position.dirX * (sprite.y - position.y));
        auto transY = invDet * (-position.planeY * (sprite.x - position.x) + position.planeX * (sprite.y - position.y));

        auto spriteScreenX = (int) ((renderWidth / 2) * (1 + transX / transY));

        auto spriteHeight = abs((int) (renderHeight / transY));
        auto dStart = std::max(-spriteHeight / 2 + renderHeight / 2, 0);
        auto dEnd = std::min(spriteHeight / 2 + renderHeight / 2, renderHeight - 1);

        auto spriteWidth = abs((int) (renderHeight / transY));
        auto dStartX = std::max(-spriteWidth / 2 + spriteScreenX, 0);
        auto dEndX = std::min(spriteWidth / 2 + spriteScreenX, renderWidth - 1);

        for (x = dStartX; x < dEndX; ++x)
        {
            if (transY > 0 && x > 0 && x < renderWidth && transY < ZBuffer[x])
                for (y = dStart; y < dEnd; ++y)
                {
                    auto texX = (int) (256 * (x - (-spriteWidth / 2 + spriteScreenX)) * TEXTURE_WIDTH / spriteWidth) / 256;
                    auto d = y * 256 - renderHeight * 128 + spriteHeight * 128;
                    auto texY = ((d * TEXTURE_HEIGHT) / spriteHeight) / 256;
                    auto color = textures[sprite.texture][TEXTURE_WIDTH * texY + texX];
                    if (color != 0x00FFFFFF)
                    {
                        if (transY > fStart && transY < fEnd)
                        {
                            auto r = (color >> 16);
                            auto g = (color >> 8) % 256;
                            auto b = color % 256;
                            r *= (1 - ((transY - fStart) / (fLength - 0.1)));
                            g *= (1 - ((transY - fStart) / (fLength)));
                            b *= (1 - ((transY - fStart) / (fLength - 0.05)));
                            if (r > 255) r = 0;
                            if (b > 255) b = 0;
                            color = 65536 * r + 256 * g + b;
                        }
                        if (transY < fEnd) buffer[x][y] = color;
                    }
                }
        }
    }

    auto pixels = (Uint32 *) (worldView->pixels);

    for (y = 0; y < renderHeight; ++y)
    {
        for (x = 0; x < renderWidth; ++x)
        {
            *pixels = buffer[x][y];
            ++pixels;
        }
        pixels += worldView->pitch / 4;
        pixels -= renderWidth;
    }

    for (x = 0; x < renderWidth; ++x) for (y = renderHeight / 3; y < 2 * renderHeight / 3; ++y) buffer[x][y] = 0;
    for (y = 0; y <= renderHeight / (flashlight ? 2.45 : 3); ++y)
    {
        color = (flashlight ? 40 : 30) - (flashlight ? 98 : 90) * y / renderHeight;
        color = 65536 * color + 256 * color + color;
        for (x = 0; x < renderWidth; ++x)
        {
            buffer[x][y] = color;
            buffer[x][renderHeight - y - 1] = color;
        }
    }
}

void Caster::draw(sdl::Surface& target)
{
    worldView.draw(target);
}
}
