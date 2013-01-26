#ifndef _UI_H_
#define _UI_H_

#include "data.h"
#include "SDL/SDL_ttf.h"

TTF_Font* uiFont;
TTF_Font* uiFontHeader;
TTF_Font* uiFontNotice;
TTF_Font* uiFontTitle;
SDL_Color uiFG;
SDL_Color uiBG;

void InitUI();
SDL_Surface* MakeWindow(int w, int h, const char* title, SDL_Rect* target, GameConfig* cfg);
SDL_Surface* MessageWindow(const char* title, const char* message, SDL_Rect* target, GameConfig* cfg);

#endif