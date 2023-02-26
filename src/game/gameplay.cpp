#include "gameplay.hpp"

#include "generator.hpp"
#include "scripting.hpp"
#include "ui.hpp"

#include <spdlog/spdlog.h>

namespace
{
constexpr auto noStateChange = std::nullopt;
}

GameplayMode::GameplayMode(UI& ui,
                           sdl::Surface& screen,
                           GameConfig& config,
                           int& noiseLevel) :
        noiseLevel(noiseLevel),
        config(config),
        player(config),
        ai(player, caster),
        ui(ui),
        scripting(*this, player, ui),
        screen(screen),
        gunHand(sdl::textures.get("gfx/gun/gun1.bmp"))
{
    LoadText((char*)texts);
}

void GameplayMode::switchFlashlight(bool target)
{
    flashlight = target;
    caster->changeVisibility(flashlight ? 2.5 : 0.5, flashlight ? 5.5 : 3.1);
}

void GameplayMode::reload()
{
    spdlog::debug("Level reload triggered");

    player.switchLevel();

    auto& level = player.currentLevel();

    auto generator = Generator(level);
    constexpr auto enableBonusRoom{true};
    generator.fillMap((int)player.getPosition().x, (int)player.getPosition().y, enableBonusRoom);
    level.npcs = generator.generateNpcs(ai);

    caster = std::make_unique<raycaster::Caster>(level);
    ai.registerSprites();
    switchFlashlight(false);
    scripting.loadScript(std::format("script/level{}.lua", player.levelId));

    player.reloadLevel = false;
}

void GameplayMode::pause()
{
    paused = true;
}

void GameplayMode::unpause()
{
    paused = false;
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
        caster->frame(player);
        if (player.revolver)
        {
            SDL_Rect gunTarget{ renderWidth / 2 - 29, renderHeight - 100, 59, 100 };
            gunHand.render(screen, gunTarget);
        }
    }
    caster->draw(screen);

    if (sdl::keyPressed(SDLK_RETURN) and paused)
    {
        ui.clear();
        paused = false;
    }

    if (not paused)
    {
        player.handleMovement(keys, frameTime);

        auto nearest = ai.distanceToNearestNpc();
        noiseLevel = (nearest < 4) ? ((nearest < 0.5) ? 128
                                                      : ceil(nearest * (nearest * (5.9242 * nearest - 39.9883) + 35.5452) + 119.484))
                                   : (!(rand() % 5) ? 2 : 1);
        if (nearest < 0.5)
        {
            return GameMode::GameOver;
        }

        if (auto textId = ai.tick(frameTime, flashlight, scripting) > 0)
        {
            ui.addMessageWindow("", texts[textId]);
            paused = true;
            return noStateChange;
        }

        if (sdl::keyPressed(SDLK_m))
        {
            auto window = ui.makeWindow(556, 556, "Mapa");
            SDL_Rect mapOffset{24, 32, 0, 0};
            player.currentLevel().drawMap(player).render(window, mapOffset);
            ui.addObject(std::move(window));
            paused = true;
            return noStateChange;
        }

        if (sdl::keyPressed(SDLK_SPACE))
        {
            player.shoot(ai);
        }
    }

    if (sdl::keyPressed(SDLK_f))
    {
        switchFlashlight(not flashlight);
    }

    if (sdl::keyPressed(SDLK_ESCAPE) or sdl::keyPressed(SDLK_q))
    {
        return GameMode::Initial;
    }

    if (flashlight and not player.battery)
    {
        switchFlashlight(false);
    }

    return noStateChange;
}
