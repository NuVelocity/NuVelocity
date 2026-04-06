#ifndef NVE_UI_WIDGET_UTILS_H
#define NVE_UI_WIDGET_UTILS_H

#include <SDL3/SDL.h>

namespace nuvelocity
{
    class Game;
    class Image;
    class SpriteBatch;
} // namespace nuvelocity

namespace nuvelocity
{
    struct BevelColors
    {
        SDL_Color light = SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255};
        SDL_Color dark = SDL_Color{.r = 96, .g = 96, .b = 96, .a = 255};
    };

    void FillRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color);
    void DrawRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color);
    void DrawBevel(SpriteBatch* batch,
                   const SDL_FRect& rect,
                   const BevelColors& colors,
                   bool sunken,
                   float thickness);

    void DrawTiledImage(SpriteBatch* spriteBatch, Image& image, const SDL_FRect& area);
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_UTILS_H
