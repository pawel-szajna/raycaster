#include "sdl.hpp"

namespace sdl
{


void Font::render(Surface& target, SDL_Rect& coords, const std::string& text)
{
    Surface textSurface(TTF_RenderUTF8_Shaded(font, text.c_str(), SDL_Color{0, 0, 0}, SDL_Color{255, 255, 255}));
    SDL_BlitSurface(*textSurface, nullptr, *target, &coords);
}

void Font::render(SDL_Surface* target, SDL_Rect& coords, const std::string& text) // TODO: delete when no longer needed
{
    Surface textSurface(TTF_RenderUTF8_Shaded(font, text.c_str(), SDL_Color{0, 0, 0}, SDL_Color{255, 255, 255}));
    SDL_BlitSurface(*textSurface, nullptr, target, &coords);
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
