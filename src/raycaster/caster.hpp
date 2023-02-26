#pragma once

#include "sdlwrapper/sdl.hpp"
#include "game/data.hpp"
#include "game/level.hpp"

#include <array>
#include <unordered_map>

class LevelInfo;
class Player;
class Position;

namespace raycaster
{
constexpr auto TEXTURE_WIDTH{120};
constexpr auto TEXTURE_HEIGHT{120};

class Caster
{
public:
    Caster(Level& level);

    void frame(const Player& player);
    void draw(sdl::Surface& target);

    void changeVisibility(double fullRange, double visibleRange);

    int addSprite(double x, double y, int texture);
    void updateSprite(int id, double x, double y, int texture);
    void removeSprite(int id);

private:
    void fadePixel(uint32_t color, double distance, uint32_t& pixel) const;

    void renderWalls(const Position& position);
    void renderSprites(const Position& position);

    void loadTexture(int id, const std::string& filename);

    double fadeStart{}, fadeEnd{};
    Level& level;

    std::unordered_map<int, std::array<Uint32, TEXTURE_WIDTH * TEXTURE_HEIGHT>> textures;
    std::array<std::array<uint32_t, renderHeight>, renderWidth> buffer{};
    std::array<double, renderWidth> zBuffer{};
    sdl::Surface worldView;
    int dynamicSpritesCounter{};

};
}
