#pragma once

#include "sdlwrapper/sdl.hpp"

#include <array>
#include <unordered_map>

class LevelInfo;
class Player;

namespace raycaster
{
constexpr auto TEXTURE_WIDTH{120};
constexpr auto TEXTURE_HEIGHT{120};

class Caster
{
public:
    Caster(int* level, const LevelInfo& li);

    void generateNoise(sdl::Surface& noise, int amount);
    void generateNoiseLinear(sdl::Surface& noise, int amount);

    void frame(const int *worldMap, const Player& player, int flashlight);
    void draw(sdl::Surface& target);

    void resetDynamicSprites();
    void addDynamicSprite(double x, double y, int texture);

private:
    void loadTexture(int id, const std::string& filename);

    std::unordered_map<int, std::array<Uint32, TEXTURE_WIDTH * TEXTURE_HEIGHT>> textures;
    sdl::Surface worldView;

};
}
