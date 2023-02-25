#include "ui.hpp"
#include "sdlwrapper/sdl.hpp"

#include <optional>
#include <string>
#include <spdlog/spdlog.h>

UI::UI(sdl::Surface &target) :
    fonts { sdl::Font("gfx/font.ttf", 12),
            sdl::Font("gfx/fontbold.ttf", 16),
            sdl::Font("gfx/hardman.ttf", 32),
            sdl::Font("gfx/hardman.ttf", 71) },
    uiFG({0, 0, 0}),
    uiBG({255, 255, 255}),
    target(target)
{
    spdlog::info("UI initialized");
}

sdl::Surface UI::messageWindow(const std::string& title, const std::string& msg, const GameConfig& cfg)
{
    SDL_Rect r = { 24, 52, 0, 0 }, p = { 24, 24, 0, 0 };

    auto window = makeWindow(480, 128, std::nullopt, cfg);

    fonts.main.render(window, r, msg);
    fonts.header.render(window, p, title);

    return window;
}

sdl::Surface UI::makeWindow(int width, int height, const std::optional<std::string>& title, const GameConfig& cfg)
{
    SDL_Rect box8px{0, 0, 8, 8};
    Sint16 x, y, w = (Sint16)width, h = (Sint16)height;

    auto window = sdl::make_surface(w, h);

    auto& textureBg = sdl::textures.get("gfx/ui/w_bg.bmp");
    for(x = 0; x < w; x += 8) for(y = 0; y < h; y += 8) textureBg.render(window, {x, y, 8, 8}, box8px);

    auto& textureTop = sdl::textures.get("gfx/ui/b_top.bmp");
    for(x = 0; x < w; x += 8) textureTop.render(window, {x, 0, 8, 8}, box8px);

    auto& textureBottom = sdl::textures.get("gfx/ui/b_down.bmp");
    for(x = 0; x < w; x += 8) textureBottom.render(window, {x, static_cast<Sint16>(h - 8), 8, 8}, box8px);

    auto& textureLeft = sdl::textures.get("gfx/ui/b_left.bmp");
    for(y = 0; y < w; y += 8) textureLeft.render(window, {0, y, 8, 8}, box8px);

    auto& textureRight = sdl::textures.get("gfx/ui/b_right.bmp");
    for(y = 0; y < w; y += 8) textureRight.render(window, {static_cast<Sint16>(w - 8), y, 8, 8}, box8px);

    sdl::textures.get("gfx/ui/b_ctl.bmp").render(window, {0, 0, 8, 8}, box8px);
    sdl::textures.get("gfx/ui/b_ctr.bmp").render(window, {static_cast<Sint16>(w - 8), 0, 8, 8}, box8px);
    sdl::textures.get("gfx/ui/b_cdl.bmp").render(window, {0, static_cast<Sint16>(h - 8), 8, 8}, box8px);;
    sdl::textures.get("gfx/ui/b_cdr.bmp").render(window, {static_cast<Sint16>(w - 8), static_cast<Sint16>(h - 8), 8, 8}, box8px);

    if (title.has_value())
    {
        SDL_Rect tgt{8, 8, 0, 0};
        fonts.header.render(window, tgt, *title);
    }

    return window;
}
