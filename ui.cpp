#include "ui.hpp"

TTF_Font* uiFont;
TTF_Font* uiFontHeader;
TTF_Font* uiFontNotice;
TTF_Font* uiFontTitle;
SDL_Color uiFG;
SDL_Color uiBG;

void InitUI()
{
	printf("Initializing UI... ");

	uiFont = TTF_OpenFont("gfx/font.ttf", 12);
	uiFontHeader = TTF_OpenFont("gfx/fontbold.ttf", 16);
	uiFontNotice = TTF_OpenFont("gfx/hardman.ttf", 32);
	uiFontTitle = TTF_OpenFont("gfx/hardman.ttf", 71);

	assert(uiFont);
	assert(uiFontHeader);
	assert(uiFontNotice);
	assert(uiFontTitle);
	
	uiFG.r = 0; uiFG.g = 0; uiFG.b = 0;
	uiBG.r = 255; uiBG.g = 255; uiBG.b = 255;

	printf("OK\n");
}

SDL_Surface* MessageWindow(const char* title, const char* msg, SDL_Rect* target, GameConfig* cfg)
{
	SDL_Surface* window;
	SDL_Surface* tmp;
	SDL_Rect r = { 24, 52, 0, 0 }, p = { 24, 24, 0, 0 };

	if(!uiFont || !uiFontHeader) InitUI();
	window = MakeWindow(480, 128, NULL, target, cfg); assert(window);
	tmp = TTF_RenderUTF8_Shaded(uiFont, msg, uiFG, uiBG);
	SDL_BlitSurface(tmp, NULL, window, &r);
	tmp = TTF_RenderUTF8_Shaded(uiFontHeader, title, uiFG, uiBG);
	SDL_BlitSurface(tmp, NULL, window, &p);
	SDL_FreeSurface(tmp);

	return window;
}

SDL_Surface* MakeWindow(int w, int h, const char* title, SDL_Rect* target, GameConfig* cfg)
{
	SDL_Surface* window;
	SDL_Surface* tmp;
	SDL_Rect r1 = { 0, 0, 8, 8 }, r2 = { 0, 0, 8, 8 };
	int x, y;

	target->w = w;
	target->h = h;
	target->x = cfg->sWidth / 2 - w / 2;
	target->y = cfg->sHeight / 2 - h / 2;

	window = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0, 0, 0, 0);
	assert(window);

	tmp = SDL_LoadBMP("gfx/ui/w_bg.bmp"); assert(tmp);
	for(x = 0; x < w; x += 8) for(y = 0; y < h; y += 8) { r1.x = x; r1.y = y; SDL_BlitSurface(tmp, &r2, window, &r1); }
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_top.bmp"); assert(tmp); r1.y = 0; 
	for(x = 0; x < w; x += 8) { r1.x = x; SDL_BlitSurface(tmp, &r2, window, &r1); }
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_down.bmp"); assert(tmp); r1.y = h - 8;
	for(x = 0; x < w; x += 8) { r1.x = x; SDL_BlitSurface(tmp, &r2, window, &r1); }
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_left.bmp"); assert(tmp); r1.x = 0;
	for(x = 0; x < w; x += 8) { r1.y = x; SDL_BlitSurface(tmp, &r2, window, &r1); }
	SDL_FreeSurface(tmp);
	
	tmp = SDL_LoadBMP("gfx/ui/b_right.bmp"); assert(tmp); r1.x = w - 8;
	for(x = 0; x < w; x += 8) { r1.y = x; SDL_BlitSurface(tmp, &r2, window, &r1); }
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_ctl.bmp"); assert(tmp);
	r1.x = 0; r1.y = 0; SDL_BlitSurface(tmp, &r2, window, &r1);
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_ctr.bmp"); assert(tmp);
	r1.x = w - 8; r1.y = 0; SDL_BlitSurface(tmp, &r2, window, &r1);
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_cdl.bmp"); assert(tmp);
	r1.x = 0; r1.y = h - 8; SDL_BlitSurface(tmp, &r2, window, &r1);
	SDL_FreeSurface(tmp);

	tmp = SDL_LoadBMP("gfx/ui/b_cdr.bmp"); assert(tmp);
	r1.x = w - 8; r1.y = h - 8; SDL_BlitSurface(tmp, &r2, window, &r1);	
	SDL_FreeSurface(tmp);

	if(title != NULL)
	{
		if(!uiFontHeader) InitUI();
		tmp = TTF_RenderUTF8_Shaded(uiFontHeader, title, uiFG, uiBG);
		r1.x = 10; r1.y = 10; SDL_BlitSurface(tmp, NULL, window, &r1);
		SDL_FreeSurface(tmp);
	}

	return window;
}