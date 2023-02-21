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

sdl::Surface messageWindow(const std::string& title, const std::string& msg, SDL_Rect* target, GameConfig* cfg)
{
	SDL_Rect r = { 24, 52, 0, 0 }, p = { 24, 24, 0, 0 };

	auto window = makeWindow(480, 128, std::nullopt, target, cfg);

    uiFont.render(window, r, msg);
    uiFontHeader.render(window, p, title);

	return window;
}

sdl::Surface makeWindow(int w, int h, const std::optional<std::string>& title, SDL_Rect* target, GameConfig* cfg)
{
	SDL_Surface* tmp;
	SDL_Rect r1 = { 0, 0, 8, 8 }, r2 = { 0, 0, 8, 8 };
	int x, y;

	target->w = w;
	target->h = h;
	target->x = cfg->sWidth / 2 - w / 2;
	target->y = cfg->sHeight / 2 - h / 2;

    auto window = sdl::make_surface(w, h);

    auto& textureBg = sdl::textures.get("gfx/ui/w_bg.bmp");
	for(x = 0; x < w; x += 8) for(y = 0; y < h; y += 8) { r1.x = x; r1.y = y; textureBg.render(window, r1, r2); }

    auto& textureTop = sdl::textures.get("gfx/ui/b_top.bmp");
	r1.y = 0;
	for(x = 0; x < w; x += 8) { r1.x = x; textureTop.render(window, r1, r2); }

	auto& textureBottom = sdl::textures.get("gfx/ui/b_down.bmp");
    r1.y = h - 8;
	for(x = 0; x < w; x += 8) { r1.x = x; textureBottom.render(window, r1, r2); }

	auto& textureLeft = sdl::textures.get("gfx/ui/b_left.bmp");
    r1.x = 0;
	for(x = 0; x < w; x += 8) { r1.y = x; textureLeft.render(window, r1, r2); }
	
	auto& textureRight = sdl::textures.get("gfx/ui/b_right.bmp");
    r1.x = w - 8;
	for(x = 0; x < w; x += 8) { r1.y = x; textureRight.render(window, r1, r2); }

	auto& textureCornerTopLeft = sdl::textures.get("gfx/ui/b_ctl.bmp");
	r1.x = 0; r1.y = 0;
    textureCornerTopLeft.render(window, r1, r2);

	auto& textureCornerTopRight = sdl::textures.get("gfx/ui/b_ctr.bmp");
	r1.x = w - 8; r1.y = 0;
    textureCornerTopRight.render(window, r1, r2);

	auto& textureCornerBottomLeft = sdl::textures.get("gfx/ui/b_cdl.bmp");
	r1.x = 0; r1.y = h - 8;
    textureCornerBottomLeft.render(window, r1, r2);

	auto& textureCornerBottomRight = sdl::textures.get("gfx/ui/b_cdr.bmp");
	r1.x = w - 8; r1.y = h - 8;
    textureCornerBottomRight.render(window, r1, r2);

	if (title.has_value())
	{
        uiFontHeader.render(window, r1, *title);
	}

	return window;
}
