#pragma once

#include "ai.hpp"
#include "player.hpp"
#include "raycaster/caster.hpp"
#include "scripting.hpp"

#include <memory>

class UI;

namespace sdl
{
class Surface;
}

class GameplayMode
{
public:
    GameplayMode(UI& ui, sdl::Surface& screen, GameConfig& config, int& noiseLevel);
    std::optional<GameMode> frame(double frameTime);

    void pause();
    void unpause();

private:
    void reload();
    void switchFlashlight(bool target);

    char texts[6][128];
    bool paused{false};
    bool flashlight{false};
    int& noiseLevel;
    GameConfig& config;
    Player player;
    AI ai;
    UI& ui;
    Scripting scripting;
    sdl::Surface& screen;
    sdl::Surface& gunHand;
    std::unique_ptr<raycaster::Caster> caster;
    std::optional<sdl::Surface> popup{};
};
