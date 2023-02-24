#pragma once

#include "data.hpp"
#include "player.hpp"
#include "sdlwrapper/sdl.hpp"

#include <functional>
#include <memory>
#include <optional>

namespace raycaster
{
class Caster;
}

class GameplayMode;

class Game
{
    struct GameState
    {
        std::function<void(void)> entryAction;
        std::function<std::optional<GameMode>(double)> step;
    };

public:

    Game();
    ~Game();

    void start();

private:

    void initializeStates();
    void changeState(GameMode target);

    void mainLoop();

    void entryInitial();
    void entryGame();
    void entryGameOver();
    std::optional<GameMode> frameInitial();
    std::optional<GameMode> frameGameOver() const;

    void applyNoise();

    GameConfig config{};
    GameMode mode{GameMode::Initial};

    sdl::Surface mainWindow;
    sdl::Surface screen;
    int noiseLevel{0};

    double stateStartTime{};

    std::unordered_map<GameMode, GameState> states;
    std::unique_ptr<GameplayMode> gameplay{nullptr};
};