#include "WidgetUtils.h"

#include <Image.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    void FillRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color)
    {
        if (batch == nullptr)
        {
            return;
        }

        batch->FillRect(&rect, color);
    }

    void DrawRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color)
    {
        if (batch == nullptr)
        {
            return;
        }

        batch->DrawLine(rect.x, rect.y, rect.x + rect.w, rect.y, color);
        batch->DrawLine(rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h, color);
        batch->DrawLine(rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h, color);
        batch->DrawLine(rect.x, rect.y + rect.h, rect.x, rect.y, color);
    }

    void DrawBevel(SpriteBatch* batch,
                   const SDL_FRect& rect,
                   const BevelColors& colors,
                   bool sunken,
                   float thickness)
    {
        if (batch == nullptr || thickness <= 0.0F)
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

        FillRect(batch, topEdge, topLeft);
        FillRect(batch, leftEdge, topLeft);
        FillRect(batch, rightEdge, bottomRight);
        FillRect(batch, bottomEdge, bottomRight);
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
                spriteBatch->Draw(image.GetSurface(), &destRect, &srcRect);
            }
        }
    }
} // namespace nuvelocity
