#pragma once

#include "data.hpp"
#include "sdl.hpp"

extern sdl::Font uiFont;
extern sdl::Font uiFontHeader;
extern sdl::Font uiFontNotice;
extern sdl::Font uiFontTitle;
extern SDL_Color uiFG;
extern SDL_Color uiBG;

void InitUI();
SDL_Surface* MakeWindow(int w, int h, const char* title, SDL_Rect* target, GameConfig* cfg);
SDL_Surface* MessageWindow(const char* title, const char* message, SDL_Rect* target, GameConfig* cfg);
