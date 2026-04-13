#include "WidgetUtils.h"
#include <StandAloneFrame.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    void WidgetUtils::FillRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color)
    {
        if (batch == nullptr)
        {
            return;
        }

        batch->FillRect(&rect, color);
    }

    void WidgetUtils::DrawRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color)
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

    void
    WidgetUtils::DrawTiledFrame(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area)
    {
        if (frame == nullptr)
        {
            return;
        }
        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        SDL_FRect srcRect{
            0.0F, 0.0F, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        DrawTiledFramePart(batch, frame, area, srcRect);
    }

    void
    WidgetUtils::DrawTiledFrameH(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area)
    {
        if (frame == nullptr)
        {
            return;
        }
        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        SDL_FRect srcRect{
            0.0F, 0.0F, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        DrawTiledFramePartH(batch, frame, area, srcRect);
    }

    void
    WidgetUtils::DrawTiledFrameV(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area)
    {
        if (frame == nullptr)
        {
            return;
        }
        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        SDL_FRect srcRect{
            0.0F, 0.0F, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        DrawTiledFramePartV(batch, frame, area, srcRect);
    }

    void WidgetUtils::DrawTiledFramePart(SpriteBatch* batch,
                                         StandAloneFrame* frame,
                                         const SDL_FRect& area,
                                         const SDL_FRect& srcRect)
    {
        if (batch == nullptr || frame == nullptr || srcRect.w <= 0 || srcRect.h <= 0)
        {
            return;
        }

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        const float tileWidth = srcRect.w;
        const float tileHeight = srcRect.h;

        for (float y = area.y; y < area.y + area.h; y += tileHeight)
        {
            for (float x = area.x; x < area.x + area.w; x += tileWidth)
            {
                const float remainingW = (area.x + area.w) - x;
                const float remainingH = (area.y + area.h) - y;

                SDL_FRect currentSrc = srcRect;
                currentSrc.w = SDL_min(tileWidth, remainingW);
                currentSrc.h = SDL_min(tileHeight, remainingH);

                SDL_FRect destRect{.x = x, .y = y, .w = currentSrc.w, .h = currentSrc.h};
                batch->Draw(surface, &destRect, &currentSrc);
            }
        }
    }

    void WidgetUtils::DrawTiledFramePartH(SpriteBatch* batch,
                                          StandAloneFrame* frame,
                                          const SDL_FRect& area,
                                          const SDL_FRect& srcRect)
    {
        if (batch == nullptr || frame == nullptr || srcRect.w <= 0 || srcRect.h <= 0)
        {
            return;
        }

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        const float tileWidth = srcRect.w;

        for (float x = area.x; x < area.x + area.w; x += tileWidth)
        {
            const float remainingW = (area.x + area.w) - x;

            SDL_FRect currentSrc = srcRect;
            currentSrc.w = SDL_min(tileWidth, remainingW);

            SDL_FRect destRect{.x = x, .y = area.y, .w = currentSrc.w, .h = currentSrc.h};
            batch->Draw(surface, &destRect, &currentSrc);
        }
    }

    void WidgetUtils::DrawTiledFramePartV(SpriteBatch* batch,
                                          StandAloneFrame* frame,
                                          const SDL_FRect& area,
                                          const SDL_FRect& srcRect)
    {
        if (batch == nullptr || frame == nullptr || srcRect.w <= 0 || srcRect.h <= 0)
        {
            return;
        }

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        const float tileHeight = srcRect.h;

        for (float y = area.y; y < area.y + area.h; y += tileHeight)
        {
            const float remainingH = (area.y + area.h) - y;

            SDL_FRect currentSrc = srcRect;
            currentSrc.h = SDL_min(tileHeight, remainingH);

            SDL_FRect destRect{.x = area.x, .y = y, .w = currentSrc.w, .h = currentSrc.h};
            batch->Draw(surface, &destRect, &currentSrc);
        }
    }

    void WidgetUtils::DrawBevel(SpriteBatch* batch,
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

        WidgetUtils::FillRect(batch, topEdge, topLeft);
        WidgetUtils::FillRect(batch, leftEdge, topLeft);
        WidgetUtils::FillRect(batch, rightEdge, bottomRight);
        WidgetUtils::FillRect(batch, bottomEdge, bottomRight);
    }
} // namespace nuvelocity
