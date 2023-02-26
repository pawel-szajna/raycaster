#include "game.hpp"

#include "SDL/SDL.h"
#include "sprig.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <format>
#include <spdlog/spdlog.h>

#include "level.hpp"
#include "raycaster/caster.hpp"
#include "ai.hpp"
#include "ui.hpp"
#include "generator.hpp"
#include "gameplay.hpp"
#include "sdlwrapper/sdl.hpp"

namespace
{
constexpr auto noStateChange = std::nullopt;
constexpr auto noiseWidth = 160;
constexpr auto noiseHeight = 90;
constexpr auto frameLimit = 7; // ms per frame max

void fillWithNoise(sdl::Surface& noise, const std::function<int()>& transparencyApplier)
{
    auto pixels = (uint32_t*)noise->pixels;
    for (auto y = 0; y < noiseHeight; ++y)
    {
        for (auto x = 0; x < noiseWidth; ++x)
        {
            auto pixel = rand() % 256;
            pixel = (pixel << 16) + (pixel << 8) + pixel;
            pixel += transparencyApplier();
            *pixels = pixel;
            ++pixels;
        }

        pixels += noise->pitch / 4;
        pixels -= noiseWidth;
    }
}
void generateNoise(sdl::Surface& noise, int intensity)
{
    fillWithNoise(noise, [intensity]()
                         {
                             if (intensity <= 112) return (rand() % (256 * intensity / 112)) << 24;
                             if (intensity == 128) return 255 << 24;
                             return ((rand() % (2048 - 16 * intensity)) + 16 * intensity - 1792) << 24;
                         });
}

void generateNoiseLinear(sdl::Surface& noise, int intensity)
{
    fillWithNoise(noise, [intensity]()
                         {
                             if (intensity <= 64) return (rand() % (4 * intensity)) << 24;
                             if (intensity == 128) return 255 << 24;
                             return ((rand() % (512 - 4 * intensity)) + 4 * intensity - 256) << 24;
                         });
}
}

Game::Game() :
    mainWindow((sdl::initialize(), sdl::make_main_window(config.sWidth, config.sHeight, config.fullScreen))),
    screen(sdl::make_surface(renderWidth, renderHeight)),
    ui(mainWindow)
{
    initializeStates();

    spdlog::debug("Game initialization complete");
}

Game::~Game()
{
    spdlog::info("Shutting down");
    sdl::teardown();
}

void Game::initializeStates()
{
    using onEntry = decltype(GameState::entryAction);
    using onFrame = decltype(GameState::step);

    states[GameMode::Initial] =  {onEntry([&]() { entryInitial(); }),              onFrame([&](double) { return frameInitial(); })};
    states[GameMode::MainMenu] = {onEntry([&]() { changeState(GameMode::Game); }), onFrame([ ](double) { return noStateChange; })};
    states[GameMode::Game] =     {onEntry([&]() { entryGame(); }),                 onFrame([&](double frameTime) { return gameplay->frame(frameTime); })};
    states[GameMode::GameOver] = {onEntry([&]() { entryGameOver(); }),             onFrame([&](double) { return frameGameOver(); })};
    states[GameMode::Quit] =     {onEntry([ ]() {}),                               onFrame([ ](double) { return noStateChange; })};
}

void Game::entryInitial()
{
    sdl::setTitle(config.title);
    noiseLevel = 128;
}

std::optional<GameMode> Game::frameInitial()
{
    SDL_Rect noticeTarget{84, 210, 0, 0};
    SDL_Rect titleTarget{0, 60, 0, 0};

    ui.fonts.title.render(screen, titleTarget, "Techdemo");
    ui.fonts.notice.render(screen, noticeTarget, "Prece enter key");

    if (noiseLevel > 12)
    {
        noiseLevel = 128 - (int)(sdl::currentTime() - stateStartTime) * 106 / 1500;
    }

    if (sdl::keyPressed(SDLK_ESCAPE) or sdl::keyPressed(SDLK_q))
    {
        return GameMode::Quit;
    }

    if (sdl::keyPressed(SDLK_RETURN))
    {
        return GameMode::Game;
    }

    return std::nullopt;
}

void Game::entryGame()
{
    spdlog::debug("Entering gameplay mode");
    gameplay = std::make_unique<GameplayMode>(ui, screen, config, noiseLevel);
}

void Game::entryGameOver()
{
    noiseLevel = 128;
}

std::optional<GameMode> Game::frameGameOver() const
{
    if (sdl::currentTime() - stateStartTime >= 1600)
    {
        return GameMode::Initial;
    }

    return noStateChange;
}

void Game::start()
{
    changeState(GameMode::Initial);
    mainLoop();
}

void Game::changeState(GameMode target)
{
    mode = target;
    stateStartTime = sdl::currentTime();
    states[mode].entryAction();
}

void Game::applyNoise()
{
    auto noise = sdl::make_alpha_surface(noiseWidth, noiseHeight);

    if (noiseLevel > 12)
    {
        generateNoiseLinear(noise, noiseLevel);
    }
    else
    {
        generateNoise(noise, 12);
    }

    auto noiseBig = sdl::transform(noise, 4);
    noiseBig.render(screen);
}

void Game::mainLoop()
{
    double newTime{}, oldTime{};

    while (mode != GameMode::Quit)
    {
        sdl::pollEvents();
        oldTime = newTime;
        newTime = sdl::currentTime();
        auto frameTime = (newTime - oldTime) / 1000;
        if (frameTime < frameLimit)
        {
            sdl::delay(frameLimit - frameTime);
        }

        screen.clear();

        auto stateChange = states[mode].step(frameTime);
        if (stateChange.has_value())
        {
            changeState(*stateChange);
        }

        applyNoise();
        screen.draw(mainWindow);
        ui.render();
        mainWindow.update();

        sdl::setTitle(std::format("{} ({} fps)", config.title, (int)(1 / frameTime)));
    }
}
