#pragma once

#include "data.hpp"
#include "sdlwrapper/sdl.hpp"

#include <optional>

class UI
{
public:
    UI(sdl::Surface& target);

    void addMessageWindow(const std::string& title, const std::string& message, const GameConfig& cfg);
    void addObject(sdl::Surface&& surface);

    sdl::Surface makeWindow(int w, int h, const std::optional<std::string>& title, const GameConfig& cfg);

    void clear();
    void render();

    struct Fonts
    {
        sdl::Font main;
        sdl::Font header;
        sdl::Font notice;
        sdl::Font title;
    } fonts;

private:

    struct UiObject
    {
        sdl::Surface surface;
        int x;
        int y;
    };

    SDL_Color uiFG;
    SDL_Color uiBG;

    sdl::Surface& target;
    std::vector<UiObject> objects;
};

