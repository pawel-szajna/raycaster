#include "sdl.hpp"
#include "sprig.h"

#include <spdlog/spdlog.h>

namespace sdl
{
TextureCache textures{}; // TODO: find a better place for this than global

void Surface::render(Surface& target)
{
    SDL_BlitSurface(surface, nullptr, *target, nullptr);
}

void Surface::render(Surface& target, SDL_Rect& coords)
{
    SDL_BlitSurface(surface, nullptr, *target, &coords);
}

void Surface::render(Surface& target, SDL_Rect coords, SDL_Rect subset)
{
    SDL_BlitSurface(surface, &subset, *target, &coords);
}

void Surface::draw(Surface& target)
{
    SDL_SoftStretch(surface, nullptr, *target, nullptr);
}

void Surface::setColorKey(uint32_t key)
{
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, key);
}

void Surface::update()
{
    SDL_UpdateRect(surface, 0, 0, 0, 0);
}

Point2D Surface::size()
{
    return Point2D{surface->clip_rect.w, surface->clip_rect.h};
}

Font::Font(const std::string& file, int size)
{
    spdlog::debug("Loading font {}", file);
    font = TTF_OpenFont(file.c_str(), size);
    if (font == nullptr)
    {
        spdlog::error("Could not load font {}", file);
        throw std::runtime_error("Could not open font file");
    }
}

void Font::render(Surface& target, SDL_Rect& coords, const std::string& text)
{
    Surface textSurface(TTF_RenderUTF8_Shaded(font, text.c_str(), SDL_Color{0, 0, 0}, SDL_Color{255, 255, 255}));
    textSurface.render(target, coords);
}

void Font::render(SDL_Surface* target, SDL_Rect& coords, const std::string& text) // TODO: delete when no longer needed
{
    Surface textSurface(TTF_RenderUTF8_Shaded(font, text.c_str(), SDL_Color{0, 0, 0}, SDL_Color{255, 255, 255}));
    SDL_BlitSurface(*textSurface, nullptr, target, &coords);
}

Surface& TextureCache::get(const std::string& path)
{
    if (not textures.contains(path))
    {
        spdlog::debug("Loading texture not in cache: {}", path);
        textures.emplace(path, Surface(SDL_LoadBMP(path.c_str())));
    }

    return textures.at(path);
}

Surface make_surface(int width, int height)
{
    return Surface(SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, 0, 0, 0, 0));
}

Surface make_alpha_surface(int width, int height)
{
    return Surface(SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000));
}

Surface make_main_window(int width, int height, bool fullScreen)
{
    return Surface(SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | (fullScreen ? SDL_FULLSCREEN : 0)));
}

Surface transform(Surface& surface, int scale)
{
    return Surface(SPG_Transform(*surface, 0, 0, (float)scale, (float)scale, 0));
}

void initialize()
{
    spdlog::info("SDL initialization");
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_ShowCursor(SDL_DISABLE);
}

void teardown()
{
    spdlog::debug("Quitting SDL");
    SDL_Quit();
}

void setTitle(const std::string& title)
{
    SDL_WM_SetCaption(title.c_str(), nullptr);
}

void delay(int ms)
{
    SDL_Delay(ms);
}

SDL_Event event{};

void pollEvents()
{
    SDL_PollEvent(&event);
}

double currentTime()
{
    return SDL_GetTicks();
}

bool keyPressed(int key)
{
    if (event.type == SDL_KEYDOWN and
        event.key.keysym.sym == key)
    {
        event.type = 0;
        return true;
    }

    return false;
}
}
