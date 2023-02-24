#pragma once

#include "sdlwrapper/sdl.hpp"
#include "game/data.hpp"

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
    Caster(int* level, const LevelInfo& li);

    void frame(const Player& player);
    void draw(sdl::Surface& target);

    void changeVisibility(double fullRange, double visibleRange);

    void resetDynamicSprites();
    void addDynamicSprite(double x, double y, int texture);

private:
    void fadePixel(uint32_t color, double distance, uint32_t& pixel) const;

    void renderWalls(const Position& position);
    void renderSprites(const Position& position);

    void loadTexture(int id, const std::string& filename);

    double fadeStart{}, fadeEnd{};
    int* worldMap;

    std::unordered_map<int, std::array<Uint32, TEXTURE_WIDTH * TEXTURE_HEIGHT>> textures;
    std::array<std::array<uint32_t, renderHeight>, renderWidth> buffer{};
    std::array<double, renderWidth> zBuffer{};
    sdl::Surface worldView;

};
}
