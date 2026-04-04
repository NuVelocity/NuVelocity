#include "WidgetUtils.h"

#include <Image.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    void FillRect(SDL_Renderer* renderer, const SDL_FRect& rect, const SDL_Color& color)
    {
        if (renderer == nullptr)
        {
            return;
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    void DrawRect(SDL_Renderer* renderer, const SDL_FRect& rect, const SDL_Color& color)
    {
        if (renderer == nullptr)
        {
            return;
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderRect(renderer, &rect);
    }

    void DrawBevel(SDL_Renderer* renderer,
                   const SDL_FRect& rect,
                   const BevelColors& colors,
                   bool sunken,
                   float thickness)
    {
        if (renderer == nullptr || thickness <= 0.0F)
        {
            return;
        }

        const SDL_Color topLeft = sunken ? colors.dark : colors.light;
        const SDL_Color bottomRight = sunken ? colors.light : colors.dark;

        SDL_FRect topEdge{.x = rect.x, .y = rect.y, .w = rect.w, .h = thickness};
        SDL_FRect leftEdge{.x = rect.x, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_FRect rightEdge{
            .x = rect.x + rect.w - thickness, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_FRect bottomEdge{
            .x = rect.x, .y = rect.y + rect.h - thickness, .w = rect.w, .h = thickness};

        FillRect(renderer, topEdge, topLeft);
        FillRect(renderer, leftEdge, topLeft);
        FillRect(renderer, rightEdge, bottomRight);
        FillRect(renderer, bottomEdge, bottomRight);
    }

    void DrawTiledImage(SpriteBatch* spriteBatch, Image& image, const SDL_FRect& area)
    {
        if (spriteBatch == nullptr || !image.IsValid())
        {
            return;
        }

        SDL_Surface* surface = image.GetSurface();
        if (surface == nullptr || surface->w <= 0 || surface->h <= 0)
        {
            return;
        }

        const float tileWidth = static_cast<float>(surface->w);
        const float tileHeight = static_cast<float>(surface->h);

        for (float y = area.y; y < area.y + area.h; y += tileHeight)
        {
            for (float x = area.x; x < area.x + area.w; x += tileWidth)
            {
                const float remainingW = (area.x + area.w) - x;
                const float remainingH = (area.y + area.h) - y;

                SDL_FRect srcRect{.x = 0.0F,
                                  .y = 0.0F,
                                  .w = SDL_min(tileWidth, remainingW),
                                  .h = SDL_min(tileHeight, remainingH)};
                SDL_FRect destRect{.x = x, .y = y, .w = srcRect.w, .h = srcRect.h};
                spriteBatch->DrawImage(image, &destRect, &srcRect);
            }
        }
    }
} // namespace nuvelocity
