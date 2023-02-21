#include "ui.hpp"
#include "sdl.hpp"

#include <optional>
#include <string>

sdl::Font uiFont;
sdl::Font uiFontHeader;
sdl::Font uiFontNotice;
sdl::Font uiFontTitle;

SDL_Color uiFG;
SDL_Color uiBG;

void InitUI()
{
    uiFont = sdl::Font("gfx/font.ttf", 12);
    uiFontHeader = sdl::Font("gfx/fontbold.ttf", 16);
    uiFontNotice = sdl::Font("gfx/hardman.ttf", 32);
    uiFontTitle = sdl::Font("gfx/hardman.ttf", 71);

	uiFG.r = 0; uiFG.g = 0; uiFG.b = 0;
	uiBG.r = 255; uiBG.g = 255; uiBG.b = 255;

	printf("OK\n");
}

sdl::Surface messageWindow(const std::string& title, const std::string& msg, SDL_Rect* target, const GameConfig& cfg)
{
	SDL_Rect r = { 24, 52, 0, 0 }, p = { 24, 24, 0, 0 };

	auto window = makeWindow(480, 128, std::nullopt, target, cfg);

    uiFont.render(window, r, msg);
    uiFontHeader.render(window, p, title);

	return window;
}

sdl::Surface makeWindow(int width, int height, const std::optional<std::string>& title, SDL_Rect* target, const GameConfig& cfg)
{
    SDL_Rect box8px{0, 0, 8, 8};
	Sint16 x, y, w = (Sint16)width, h = (Sint16)height;

	target->w = w;
	target->h = h;
	target->x = cfg.sWidth / 2 - w / 2;
	target->y = cfg.sHeight / 2 - h / 2;

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

	auto& textureCornerTopLeft = sdl::textures.get("gfx/ui/b_ctl.bmp");
    textureCornerTopLeft.render(window, {0, 0, 8, 8}, box8px);

	auto& textureCornerTopRight = sdl::textures.get("gfx/ui/b_ctr.bmp");
    textureCornerTopRight.render(window, {static_cast<Sint16>(w - 8), 0, 8, 8}, box8px);

	auto& textureCornerBottomLeft = sdl::textures.get("gfx/ui/b_cdl.bmp");
    textureCornerBottomLeft.render(window, {0, static_cast<Sint16>(h - 8), 8, 8}, box8px);

	auto& textureCornerBottomRight = sdl::textures.get("gfx/ui/b_cdr.bmp");
    textureCornerBottomRight.render(window, {static_cast<Sint16>(w - 8), static_cast<Sint16>(h - 8), 8, 8}, box8px);

	if (title.has_value())
	{
        SDL_Rect tgt{8, 8, 0, 0};
        uiFontHeader.render(window, tgt, *title);
	}

	return window;
}
