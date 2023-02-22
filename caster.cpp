#include "caster.hpp"

#include "SDL/SDL.h"

#include "player.hpp"
#include "level.hpp"
#include "sdl.hpp"

#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

double ZBuffer[wwWidth];

Uint32 buffer[wwWidth][wwHeight];
Uint32 textures[texCount][texWidth * texHeight];
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

void LoadTexture(Uint32* memory, const char* filename)
{
    FILE* data = fopen(filename, "rb");
    if (not data)
    {
        spdlog::warn("File not found: {}!", filename);
    }

    unsigned char datar, datag, datab;

    for (int y = 0; y < (texWidth * texHeight); ++y)
    {
        fread(&datar, sizeof(char), 1, data);
        fread(&datag, sizeof(char), 1, data);
        fread(&datab, sizeof(char), 1, data);
        memory[y] = 65536 * datar + 256 * datag + datab;
    }
    fclose(data);
}

sdl::Surface InitCaster(int* level, LevelInfo* li)
{
    int x, y, u;
    char filename[128];

    spdlog::info("Initializing caster");
    assert(level);
    assert(li);

    auto wv = sdl::make_surface(wwWidth, wwHeight);

    auto numSprites = 0;
    for (x = 0; x < levelSize; ++x)
        for (y = 0; y < levelSize; ++y)
            if (BlockType(level, x, y) == 2 || BlockType(level, x, y) == 5 || BlockType(level, x, y) == 6)
                ++numSprites;

    sprites.clear();
    sprites.reserve(numSprites);

    for (x = 0; x < levelSize; ++x) for (y = 0; y < levelSize; ++y)
    {
        spdlog::info("Adding sprite");
        u = BlockType(level, x, y);
        if(u == 2 || u == 5 || u == 6)
        {
            auto texture = ((level[levelSize * x + y] / 16) % 16) + (u == 2 ? 16 : (u == 5 ? 32 : 0));
            sprites.push_back(Sprite{0.5 + x, 0.5 + y, texture, 0});
        }
    }

    spdlog::info("Loading textures");

    for(x = 1; x <= (li->textureSetSize); ++x)
    {
        sprintf(filename, "gfx/set_%d/%d.raw", li->textureSet, x);
        LoadTexture(textures[x], filename);
    }

    for(x = 0; x < li->pillarTexCount; ++x)
    {
        sprintf(filename, "gfx/%s.raw", li->pillarTex[x]);
        LoadTexture(textures[16+x], filename);
    }

    for(x = 0; x < li->debrisTexCount; ++x)
    {
        sprintf(filename, "gfx/%s.raw", li->debrisTex[x]);
        LoadTexture(textures[32+x], filename);
    }

    for(x = 0; x < li->npcTexCount; ++x)
    {
        sprintf(filename, "gfx/%s_walk1.raw", li->npcTex[x]); LoadTexture(textures[48+x], filename);
        sprintf(filename, "gfx/%s_walk2.raw", li->npcTex[x]); LoadTexture(textures[56+x], filename);
        sprintf(filename, "gfx/%s_walk3.raw", li->npcTex[x]); LoadTexture(textures[64+x], filename);
        sprintf(filename, "gfx/%s_walk4.raw", li->npcTex[x]); LoadTexture(textures[72+x], filename);
        sprintf(filename, "gfx/%s_shoot.raw", li->npcTex[x]); LoadTexture(textures[80+x], filename);
        sprintf(filename, "gfx/%s_dead.raw", li->npcTex[x]); LoadTexture(textures[88+x], filename);
    }

    for(x = 0; x < 4; ++x)
    {
        sprintf(filename, "gfx/items/%d.raw", x); LoadTexture(textures[96+x], filename);
    }

    LoadTexture(textures[0], "gfx/transparent.raw");
    LoadTexture(textures[10], "gfx/portal.raw");

    return wv;
}

void ResetDynamicSprites()
{
    spdlog::debug("Reset dynamic sprites");
    sprites.erase(std::remove_if(sprites.begin(), sprites.end(), [](const auto& sprite) { return sprite.dynamic; }),
                  sprites.end());
}

void AddDynamicSprite(double x, double y, int texture)
{
    spdlog::debug("Add dynamic sprite @ {};{}", x, y);
    sprites.push_back(Sprite{x, y, texture, 0, DYNAMIC});
}

void CastFrame(SDL_Surface* worldview, int* worldMap, Player* player, int flashlight)
{
    int x, y;

    auto fStart = flashlight ? 2.5 : 0.5;
    auto fEnd = flashlight ? 5.5 : 3.1;
    auto fLength = fEnd - fStart;
    
    const auto& position = player->getPosition() ;

    for(x = 0; x < wwWidth; ++x)
    {
        auto camX = 2 * x / (double)wwWidth - 1; /* wspolrzedna x na plaszczyznie na ktorej rzutowany jest obraz */
        auto rayPX = position.x;
        auto rayPY = position.y;
        auto rayDX = position.dirX + position.planeX * camX;
        auto rayDY = position.dirY + position.planeY * camX;
        auto mapX = (int)rayPX;
        auto mapY = (int)rayPY;
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
        auto lineHeight = abs((int)(wwHeight / wallDist));
        auto dStart = std::max(-lineHeight / 2 + wwHeight / 2, 0);
        auto dEnd = std::min(lineHeight / 2 + wwHeight / 2, wwHeight - 1);

        auto tex = (worldMap[mapX * levelSize + mapY] >> 4);
        if (side == Hit::Horizontal and position.x < mapX)
        {
            tex >>= 8;
        }
        if (side == Hit::Vertical)
        {
            if(position.y < mapY) tex >>= 12;
            else tex >>= 4;
        }
        tex %= 16;

        auto wallX = side == Hit::Vertical
                   ? rayPX + ((mapY - rayPY + (1 - stepY) / 2) / rayDY) * rayDX
                   : rayPY + ((mapX - rayPX + (1 - stepX) / 2) / rayDX) * rayDY;
        wallX -= floor(wallX);
        auto texX = (int)(wallX * (double)texWidth);
        if ((side == Hit::Horizontal and rayDX > 0) or (side == Hit::Vertical && rayDY < 0))
        {
            texX = texWidth - texX - 1;
        }

        for (y = dStart; y < dEnd; ++y)
        {
            auto d = y * 256 - wwHeight * 128 + lineHeight * 128;
            auto texY = ((d * texHeight) / lineHeight) / 256;
            color = textures[tex][texHeight * texY + texX];
            if(wallDist > fStart && wallDist < fEnd)
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
            if(wallDist < fEnd) buffer[x][y] = color;
        }

        ZBuffer[x] = wallDist;
    }

    std::transform(sprites.begin(), sprites.end(), sprites.begin(),
                   [&position](auto sprite)
                   {
                       sprite.distance = sqr(position.x - sprite.x) + sqr(position.y - sprite.y);
                       return sprite;
                   });

    std::sort(sprites.begin(), sprites.end(),
              [](const auto &a, const auto &b) { return a.distance < b.distance; });

    for (const auto& sprite : sprites)
    {
        auto invDet = 1.0 / (position.planeX * position.dirY - position.dirX * position.planeY);

        auto transX = invDet * (position.dirY * (sprite.x - position.x) - position.dirX * (sprite.y - position.y));
        auto transY = invDet * (-position.planeY * (sprite.x - position.x) + position.planeX * (sprite.y - position.y));

        auto spriteScreenX = (int)((wwWidth / 2) * (1 + transX / transY));

        auto spriteHeight = abs((int)(wwHeight / transY));
        auto dStart = std::max(-spriteHeight / 2 + wwHeight / 2, 0);
        auto dEnd = std::min(spriteHeight / 2 + wwHeight / 2, wwHeight - 1);

        auto spriteWidth = abs((int)(wwHeight / transY));
        auto dStartX = std::max(-spriteWidth / 2 + spriteScreenX, 0);
        auto dEndX = std::min(spriteWidth / 2 + spriteScreenX, wwWidth - 1);

        for (x = dStartX; x < dEndX; ++x)
        {
            if (transY > 0 && x > 0 && x < wwWidth && transY < ZBuffer[x]) for(y = dStart; y < dEnd; ++y)
            {
                auto texX = (int)(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
                auto d = y * 256 - wwHeight * 128 + spriteHeight * 128;
                auto texY = ((d * texHeight) / spriteHeight) / 256;
                color = textures[sprite.texture][texWidth * texY + texX];
                if(color != 0x00FFFFFF)
                {
                    if(transY > fStart && transY < fEnd)
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
                    if(transY < fEnd) buffer[x][y] = color;
                }
            }
        }
    }

    auto pixels = (Uint32*)(worldview->pixels);

    for (y = 0; y < wwHeight; ++y)
    {
        for (x = 0; x < wwWidth; ++x)
        {
            *pixels = buffer[x][y];
            ++pixels;
        }
        pixels += worldview->pitch / 4;
        pixels -= wwWidth;
    }

    for (x = 0; x < wwWidth; ++x) for(y = wwHeight/3; y < 2*wwHeight/3; ++y) buffer[x][y] = 0;
    for (y = 0; y <= wwHeight / (flashlight ? 2.45 : 3); ++y)
    {
        color = (flashlight ? 40 : 30) - (flashlight ? 98 : 90) * y / wwHeight;
        color = 65536 * color + 256 * color + color;
        for(x = 0; x < wwWidth; ++x)
        {
            buffer[x][y] = color;
            buffer[x][wwHeight - y - 1] = color;
        }
    }
}

void generateNoise(sdl::Surface& noise, int amount)
{
    int x, y, buf;

    if(amount < 1) amount = 1;

    auto pixels = (Uint32*)(noise->pixels);
    for(y = 0; y < 90; ++y)
    {
        for(x = 0; x < 160; ++x)
        {
            buf = rand() % 256;
            buf = (buf << 16) + (buf << 8) + (buf);
            if(amount <= 112) buf+= (rand() % (256 * amount / 112)) << 24;
            else if(amount == 128) buf+= 0xff000000;
            else buf+= ((rand() % (2048 - 16 * amount)) + 16 * amount - 1792) << 24;
            *pixels = buf;
            ++pixels;
        }
        pixels += noise->pitch / 4;
        pixels -= 160;
    }
}

void generateNoiseLinear(sdl::Surface& noise, int amount)
{
    int x, y, buf;

    if(amount < 1) amount = 1;

    auto pixels = (Uint32*)(noise->pixels);
    for(y = 0; y < 90; ++y)
    {
        for(x = 0; x < 160; ++x)
        {
            buf = rand() % 256;
            buf = (buf << 16) + (buf << 8) + (buf);
            if(amount <= 64) buf+= (rand() % (4 * amount)) << 24;
            else if(amount == 128) buf+= 0xff000000;
            else buf+= ((rand() % (512 - 4 * amount)) + 4 * amount - 256) << 24;
            *pixels = buf;
            ++pixels;
        }
        pixels += noise->pitch / 4;
        pixels -= 160;
    }
}
