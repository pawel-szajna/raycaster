#pragma once

#include "data.hpp"
#include "sdlwrapper/sdl.hpp"

#include <optional>

extern sdl::Font uiFont;
extern sdl::Font uiFontHeader;
extern sdl::Font uiFontNotice;
extern sdl::Font uiFontTitle;
extern SDL_Color uiFG;
extern SDL_Color uiBG;

void InitUI();
sdl::Surface makeWindow(int w, int h, const std::optional<std::string>& title, SDL_Rect* target, const GameConfig& cfg);
sdl::Surface messageWindow(const std::string& title, const std::string& message, SDL_Rect* target, const GameConfig& cfg);
