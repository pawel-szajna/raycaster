#ifndef _UI_H_
#define _UI_H_

#include "data.h"
#include "SDL/SDL_ttf.h"

extern TTF_Font* uiFont;
extern TTF_Font* uiFontHeader;
extern TTF_Font* uiFontNotice;
extern TTF_Font* uiFontTitle;
extern SDL_Color uiFG;
extern SDL_Color uiBG;

void InitUI();
SDL_Surface* MakeWindow(int w, int h, const char* title, SDL_Rect* target, GameConfig* cfg);
SDL_Surface* MessageWindow(const char* title, const char* message, SDL_Rect* target, GameConfig* cfg);

#endif