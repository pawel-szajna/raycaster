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
#include "sdlwrapper/sdl.hpp"

namespace
{
constexpr auto noStateChange = std::nullopt;
constexpr auto noiseWidth = 160;
constexpr auto noiseHeight = 90;

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

class GameplayMode
{
public:
    GameplayMode(sdl::Surface& screen, GameConfig& config, int& noiseLevel);
    std::optional<GameMode> frame(double frameTime);

private:
    void reload();

    int level[levelSize][levelSize];
    char visited[levelSize][levelSize];
    char texts[6][128];
    bool paused{false};
    bool flashlight{false};
    int& noiseLevel;
    GameConfig& config;
    LevelInfo levelInfo{};
    NPCs npcs{};
    Player player;
    sdl::Surface& screen;
    sdl::Surface& gunHand;
    std::unique_ptr<raycaster::Caster> caster;
    std::optional<sdl::Surface> popup{};
};

GameplayMode::GameplayMode(sdl::Surface& screen,
                           GameConfig& config,
                           int& noiseLevel) :
    noiseLevel(noiseLevel),
    config(config),
    player(config),
    screen(screen),
    gunHand(sdl::textures.get("gfx/gun/gun1.bmp"))
{
    LoadText((char*)texts);
}

void GameplayMode::reload()
{
    spdlog::debug("Level reload triggered");

    npcs = generate_npcs((int*)level);
    InitAI((int*)level);
    InitUI();

    player.switchLevel();
    generate_map((int*)level, (int)player.getPosition().x, (int)player.getPosition().y, 1);
    npcs = player.currentLevel().npcs;
    ResetAI(npcs);

    player.currentLevel().addItem(27, 46, 0);
    player.currentLevel().addItem(27, 47, 1);
    player.currentLevel().addItem(27, 48, 3);

    caster = std::make_unique<raycaster::Caster>((int*)level, player.currentLevel().li);

    player.reloadLevel = false;
}

std::optional<GameMode> GameplayMode::frame(double frameTime)
{
    if (player.reloadLevel)
    {
        reload();
    }

    auto keys = SDL_GetKeyState(nullptr);

    if (not paused)
    {
        caster->frame(player, flashlight and player.blink());
        if (player.revolver)
        {
            SDL_Rect gunTarget{ renderWidth / 2 - 29, renderHeight - 100, 59, 100 };
            gunHand.render(screen, gunTarget);
        }
    }
    caster->draw(screen);

    if (popup.has_value())
    {
        auto [popupWidth, popupHeight] = popup->size();
        auto [screenWidth, screenHeight] = screen.size();
        SDL_Rect popupPosition{static_cast<int16_t>(screenWidth / 2 - popupWidth / 2),
                               static_cast<int16_t>(screenHeight / 2 - popupHeight / 2),
                               static_cast<uint16_t>(popupWidth),
                               static_cast<uint16_t>(popupHeight)};
        popup->render(screen, popupPosition);
    }

    if (sdl::keyPressed(SDLK_RETURN) and popup.has_value())
    {
        popup = std::nullopt;
        paused = false;
    }

    if (not paused)
    {
        player.handleMovement(keys, (int*) level, (char*) visited, frameTime);

        auto nearest = AI_DistanceToNearestNPC(&player);
        noiseLevel = (nearest < 4) ? ((nearest < 0.5) ? 128
                                                      : ceil(nearest * (nearest * (5.9242 * nearest - 39.9883) + 35.5452) + 119.484))
                                   : (!(rand() % 5) ? 2 : 1);
        if (nearest < 0.5)
        {
            return GameMode::GameOver;
        }

        if (auto textId = AI_Tick(&player, frameTime, flashlight) > 0)
        {
            popup = messageWindow("", texts[textId], config);
            paused = true;
            return noStateChange;
        }

        if (sdl::keyPressed(SDLK_m))
        {
            popup = makeWindow(556, 556, "Mapa", config);
            drawMap((int*) level, player).render(*popup);
            paused = true;
            return noStateChange;
        }

        if (sdl::keyPressed(SDLK_SPACE))
        {
            player.shoot((int*)level);
        }
    }

    if (sdl::keyPressed(SDLK_f))
    {
        flashlight = not flashlight;
        caster->changeVisibility(flashlight ? 2.5 : 0.5, flashlight ? 5.5 : 3.1);
    }

    if (sdl::keyPressed(SDLK_ESCAPE) or sdl::keyPressed(SDLK_q))
    {
        return GameMode::Initial;
    }

    if (flashlight and not player.battery)
    {
        flashlight = false;
    }

    return noStateChange;
}

Game::Game() :
    mainWindow((sdl::initialize(), sdl::make_main_window(config.sWidth, config.sHeight, config.fullScreen))),
    screen(sdl::make_surface(renderWidth, renderHeight))
{
    InitUI();
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
}

std::optional<GameMode> Game::frameInitial()
{
    SDL_Rect noticeTarget{84, 210, 0, 0};
    SDL_Rect titleTarget{0, 60, 0, 0};

    uiFontTitle.render(screen, titleTarget, "Techdemo");
    uiFontNotice.render(screen, noticeTarget, "Prece enter key");

    if (noiseLevel > 12)
    {
        noiseLevel -= 2;
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
    gameplay = std::make_unique<GameplayMode>(screen, config, noiseLevel);
}

void Game::entryGameOver()
{
    gameOverStart = sdl::currentTime();
    noiseLevel = 128;
}

std::optional<GameMode> Game::frameGameOver() const
{
    if (sdl::currentTime() - gameOverStart >= 1600)
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

        auto stateChange = states[mode].step(frameTime);
        if (stateChange.has_value())
        {
            changeState(*stateChange);
        }

        applyNoise();
        screen.draw(mainWindow);
        mainWindow.update();

        sdl::delay(15);
        sdl::setTitle(std::format("{} ({} fps)", config.title, (int)(1 / frameTime)));
    }
}
