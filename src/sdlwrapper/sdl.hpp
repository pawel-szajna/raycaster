#pragma once

#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#include <string>
#include <stdexcept>
#include <unordered_map>

namespace sdl
{
class Surface
{
public:
    explicit Surface(SDL_Surface* source) : surface(source) {}

    Surface() = delete;
    Surface(const Surface& other) = delete;
    Surface& operator= (const Surface& other) = delete;

    ~Surface() { SDL_free(surface); }

    Surface(Surface&& other) noexcept : surface(other.surface) { other.surface = nullptr; }
    Surface& operator=(Surface&& other) noexcept { std::swap(surface, other.surface); return *this; }

    SDL_Surface * operator*() { return surface; }
    SDL_Surface * operator->() { return surface; }
    const SDL_Surface * operator*() const { return surface; }
    const SDL_Surface * operator->() const { return surface; }

    void render(Surface& target, SDL_Rect& coords);
    void render(Surface& target, SDL_Rect coords, SDL_Rect subset);
    void draw(Surface& target);

private:
    SDL_Surface* surface{nullptr};
};

class Font
{
public:
    explicit Font(TTF_Font* source) : font(source) {}
    Font(const std::string& file, int size);

    Font() = default; // TODO: "=delete" me when code is ready
    Font(const Font& other) = delete;
    Font& operator= (const Font& other) = delete;

    ~Font() { TTF_CloseFont(font); }

    Font(Font&& other) noexcept : font(other.font) { other.font = nullptr; }
    Font& operator=(Font&& other) noexcept { std::swap(font, other.font); return *this; }

    TTF_Font* operator*() { return font; }
    TTF_Font* operator->() { return font; }
    const TTF_Font* operator*() const { return font; }
    const TTF_Font* operator->() const { return font; }

    void render(Surface& target, SDL_Rect& coords, const std::string& text);
    void render(SDL_Surface* target, SDL_Rect& coords, const std::string& text); // TODO: delete when no longer needed

private:
    TTF_Font* font{nullptr};
};

class TextureCache
{
public:
    Surface& get(const std::string& path);

private:
    std::unordered_map<std::string, Surface> textures{};
};

extern TextureCache textures;

Surface make_surface(int width, int height);
Surface make_alpha_surface(int width, int height);

void initialize();
void teardown();
}
