#include "sdl.hpp"

namespace sdl
{
TextureCache textures{}; // TODO: find a better place for this than global

void Surface::render(Surface& target, SDL_Rect& coords)
{
    SDL_BlitSurface(surface, nullptr, *target, &coords);
}

void Surface::render(SDL_Surface* target, SDL_Rect& coords)
{
    SDL_BlitSurface(surface, nullptr, target, &coords);
}

void Surface::render(Surface& target, SDL_Rect& coords, SDL_Rect& subset)
{
    SDL_BlitSurface(surface, &subset, *target, &coords);
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
        printf("Loading texture not in cache: %s", path.c_str());
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
}
