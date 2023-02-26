#include "caster.hpp"

#include "game/player.hpp"
#include "game/level.hpp"
#include "sdlwrapper/sdl.hpp"

#include <algorithm>
#include <cmath>
#include <execution>
#include <ranges>
#include <spdlog/spdlog.h>

namespace raycaster
{
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

constexpr auto sqr(auto x) { return x * x; }

constexpr auto outOfBounds(int x, int y, int size) { return x < 0 or y < 0 or x >= size or y >= size; }

constexpr auto color(uint32_t red, uint32_t green, uint32_t blue) { return 65536 * red + 256 * green + blue; }
constexpr auto gray(uint32_t brightness) { return color(brightness, brightness, brightness); }

namespace tile
{
constexpr auto type(int tile) { return tile % 16; }
constexpr auto texture(int tile) { return (tile / 16) % 16; }
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

Caster::Caster(Level& level) :
    level(level),
    worldView(sdl::make_surface(renderWidth, renderHeight))
{
    spdlog::info("Initializing caster");

    auto numSprites = std::count_if(level.map.begin(), level.map.end(),
                                    [](auto tile) { return tile::type(tile) == 2 or tile::type(tile) == 5 or tile::type(tile) == 6; });

    sprites.clear();
    sprites.reserve(numSprites);

    for (int x = 0; x < levelSize; ++x)
        for (int y = 0; y < levelSize; ++y)
        {
            int type = tile::type(level.at(x, y));
            if (type == 2 || type == 5 || type == 6)
            {
                auto texture = tile::texture(level.at(x, y)) + (type == 2 ? 16 : (type == 5 ? 32 : 0));
                sprites.push_back(Sprite{0.5 + x, 0.5 + y, texture, 0});
            }
        }

    spdlog::info("Loading textures");

    for (int x = 1; x <= level.li.textureSetSize; ++x) loadTexture(x, std::format("gfx/set_{}/{}.raw", level.li.textureSet, x));
    for (int x = 0; x < level.li.pillarTexCount; ++x) loadTexture(16 + x, std::format("gfx/{}.raw", level.li.pillarTex[x]));
    for (int x = 0; x < level.li.debrisTexCount; ++x) loadTexture(32 + x, std::format("gfx/{}.raw", level.li.debrisTex[x]));
    for (int x = 0; x < 4; ++x) loadTexture(96 + x, std::format("gfx/items/{}.raw", x));

    for (int x = 0; x < level.li.npcTexCount; ++x)
    {
        loadTexture(48 + x, std::format("gfx/{}_walk1.raw", level.li.npcTex[x]));
        loadTexture(56 + x, std::format("gfx/{}_walk2.raw", level.li.npcTex[x]));
        loadTexture(64 + x, std::format("gfx/{}_walk3.raw", level.li.npcTex[x]));
        loadTexture(72 + x, std::format("gfx/{}_walk4.raw", level.li.npcTex[x]));
        loadTexture(80 + x, std::format("gfx/{}_shoot.raw", level.li.npcTex[x]));
        loadTexture(88 + x, std::format("gfx/{}_dead.raw", level.li.npcTex[x]));
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

void Caster::changeVisibility(double fullRange, double visibleRange)
{
    fadeStart = fullRange;
    fadeEnd = visibleRange;
}

void Caster::frame(const Player& player)
{
    for (auto y = 0; y < renderHeight / 2; ++y)
    {
        auto color = gray(std::max(0, 30 - 90 * y / renderHeight));
        for (auto x = 0; x < renderWidth; ++x)
        {
            buffer[x][y] = color;
            buffer[x][renderHeight - y - 1] = color;
        }
    }

    const auto& position = player.getPosition();

    renderWalls(position);
    renderSprites(position);

    auto pixels = (Uint32*)(worldView->pixels);

    for (auto y = 0; y < renderHeight; ++y)
    {
        for (auto x = 0; x < renderWidth; ++x)
        {
            *pixels = buffer[x][y];
            ++pixels;
        }
        pixels += worldView->pitch / 4;
        pixels -= renderWidth;
    }
}

void Caster::fadePixel(uint32_t color, double distance, uint32_t& pixel) const
{
    if (distance >= fadeEnd)
    {
        pixel = 0;
        return;
    }

    if (distance > fadeStart)
    {
        auto r = ((int32_t)color >> 16) % 256;
        auto g = ((int32_t)color >> 8) % 256;
        auto b = (int32_t)color % 256;
        auto fadeLength = fadeEnd - fadeStart;
        r *= (1 - ((distance - fadeStart) / (fadeLength - 0.1)));
        g *= (1 - ((distance - fadeStart) / (fadeLength)));
        b *= (1 - ((distance - fadeStart) / (fadeLength - 0.05)));
        color = std::max(r, 0) * 65536
              + std::max(g, 0) * 256
              + std::max(b, 0);
    }

    pixel = color;
}

void Caster::renderWalls(const Position& position)
{
    auto columns = std::ranges::views::iota(0, renderWidth);
    std::transform(std::execution::par,
                   std::cbegin(columns),
                   std::cend(columns),
                   std::begin(buffer),
                   [&] (auto x)
                   {
                       decltype(buffer)::value_type column{};

                       auto camX = 2 * x / (double)renderWidth - 1;
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

                           if (tile::type(level.at(mapX, mapY)) == 1)
                           {
                               break;
                           }
                       }

                       if (outOfBounds(mapX, mapY, levelSize))
                       {
                           return column;
                       }

                       auto wallDist = side == Hit::Vertical
                                     ? fabs((mapY - rayPY + (1 - stepY) / 2) / rayDY)
                                     : fabs((mapX - rayPX + (1 - stepX) / 2) / rayDX);

                       if (wallDist > fadeEnd)
                       {
                           zBuffer[x] = fadeEnd;
                           return column;
                       }

                       auto lineHeight = abs((int) (renderHeight / wallDist));
                       auto dStart = std::max(-lineHeight / 2 + renderHeight / 2, 0);
                       auto dEnd = std::min(lineHeight / 2 + renderHeight / 2, renderHeight - 1);

                       auto tex = (level.at(mapX, mapY) >> 4);
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

                       for (auto y = dStart; y < dEnd; ++y)
                       {
                           auto d = y * 256 - renderHeight * 128 + lineHeight * 128;
                           auto texY = ((d * TEXTURE_HEIGHT) / lineHeight) / 256;
                           auto color = textures[tex][TEXTURE_HEIGHT * texY + texX];
                           fadePixel(color, wallDist, column[y]);
                       }

                       zBuffer[x] = wallDist;
                       return column;
                   });
}

void Caster::renderSprites(const Position& position)
{
    std::transform(sprites.begin(), sprites.end(), sprites.begin(),
                   [&position](auto& sprite) {
                       sprite.distance = sqr(position.x - sprite.x) + sqr(position.y - sprite.y);
                       return sprite;
                   });

    std::sort(std::execution::par,
              sprites.begin(), sprites.end(),
              [](const auto& a, const auto& b) { return a.distance < b.distance; });

    for (const auto &sprite: sprites)
    {
        if (sprite.distance > (fadeEnd * fadeEnd))
        {
            continue;
        }

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

        for (auto x = dStartX; x < dEndX; ++x)
        {
            if (transY > 0 && x > 0 && x < renderWidth && transY < zBuffer[x])
                for (auto y = dStart; y < dEnd; ++y)
                {
                    auto texX = (int) (256 * (x - (-spriteWidth / 2 + spriteScreenX)) * TEXTURE_WIDTH / spriteWidth) / 256;
                    auto d = y * 256 - renderHeight * 128 + spriteHeight * 128;
                    auto texY = ((d * TEXTURE_HEIGHT) / spriteHeight) / 256;
                    auto color = textures[sprite.texture][TEXTURE_WIDTH * texY + texX];
                    if (color != 0x00FFFFFF)
                    {
                        fadePixel(color, transY, buffer[x][y]);
                    }
                }
        }
    }
}

void Caster::draw(sdl::Surface& target)
{
    worldView.draw(target);
}
}
