#pragma once

#include "data.hpp"
#include "sdlwrapper/sdl.hpp"

#include <optional>

class UI
{
public:
    UI(sdl::Surface& target);

    sdl::Surface makeWindow(int w, int h, const std::optional<std::string>& title, const GameConfig& cfg);
    sdl::Surface messageWindow(const std::string& title, const std::string& message, const GameConfig& cfg);

    struct Fonts
    {
        sdl::Font main;
        sdl::Font header;
        sdl::Font notice;
        sdl::Font title;
    } fonts;

private:
    SDL_Color uiFG;
    SDL_Color uiBG;

    sdl::Surface& target;
};

