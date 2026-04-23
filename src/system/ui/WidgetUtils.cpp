#include "WidgetUtils.h"
#include <StandAloneFrame.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    void WidgetUtils::FillRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color)
    {
        if (batch == nullptr)
        {
            return;
        }

        batch->FillRect(&rect, color);
    }

    void WidgetUtils::DrawRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color)
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
    WidgetUtils::DrawTiledFrame(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area)
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

        SDL_Rect srcRect{0, 0, surface->w, surface->h};
        DrawTiledFramePart(batch, frame, area, srcRect);
    }

    void
    WidgetUtils::DrawTiledFrameH(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area)
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

        SDL_Rect srcRect{0, 0, surface->w, surface->h};
        DrawTiledFramePartH(batch, frame, area, srcRect);
    }

    void
    WidgetUtils::DrawTiledFrameV(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area)
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

        SDL_Rect srcRect{0, 0, surface->w, surface->h};
        DrawTiledFramePartV(batch, frame, area, srcRect);
    }

    void WidgetUtils::DrawTiledFramePart(SpriteBatch* batch,
                                         StandAloneFrame* frame,
                                         const SDL_Rect& area,
                                         const SDL_Rect& srcRect)
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

        const int tileWidth = srcRect.w;
        const int tileHeight = srcRect.h;

        for (int y = area.y; y < area.y + area.h; y += tileHeight)
        {
            for (int x = area.x; x < area.x + area.w; x += tileWidth)
            {
                const int remainingW = (area.x + area.w) - x;
                const int remainingH = (area.y + area.h) - y;

                SDL_Rect currentSrc = srcRect;
                currentSrc.w = SDL_min(tileWidth, remainingW);
                currentSrc.h = SDL_min(tileHeight, remainingH);

                SDL_Rect destRect{.x = x, .y = y, .w = currentSrc.w, .h = currentSrc.h};
                batch->Draw(surface, &destRect, &currentSrc);
            }
        }
    }

    void WidgetUtils::DrawTiledFramePartH(SpriteBatch* batch,
                                          StandAloneFrame* frame,
                                          const SDL_Rect& area,
                                          const SDL_Rect& srcRect)
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

        const int tileWidth = srcRect.w;

        for (int x = area.x; x < area.x + area.w; x += tileWidth)
        {
            const int remainingW = (area.x + area.w) - x;

            SDL_Rect currentSrc = srcRect;
            currentSrc.w = SDL_min(tileWidth, remainingW);

            SDL_Rect destRect{.x = x, .y = area.y, .w = currentSrc.w, .h = currentSrc.h};
            batch->Draw(surface, &destRect, &currentSrc);
        }
    }

    void WidgetUtils::DrawTiledFramePartV(SpriteBatch* batch,
                                          StandAloneFrame* frame,
                                          const SDL_Rect& area,
                                          const SDL_Rect& srcRect)
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

        const int tileHeight = srcRect.h;

        for (int y = area.y; y < area.y + area.h; y += tileHeight)
        {
            const int remainingH = (area.y + area.h) - y;

            SDL_Rect currentSrc = srcRect;
            currentSrc.h = SDL_min(tileHeight, remainingH);

            SDL_Rect destRect{.x = area.x, .y = y, .w = currentSrc.w, .h = currentSrc.h};
            batch->Draw(surface, &destRect, &currentSrc);
        }
    }

    void WidgetUtils::DrawBevel(SpriteBatch* batch,
                                const SDL_Rect& rect,
                                const BevelColors& colors,
                                bool sunken,
                                int thickness)
    {
        if (batch == nullptr || thickness <= 0)
        {
            return;
        }

        const SDL_Color topLeft = sunken ? colors.dark : colors.light;
        const SDL_Color bottomRight = sunken ? colors.light : colors.dark;

        SDL_Rect topEdge{.x = rect.x, .y = rect.y, .w = rect.w, .h = thickness};
        SDL_Rect leftEdge{.x = rect.x, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_Rect rightEdge{
            .x = rect.x + rect.w - thickness, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_Rect bottomEdge{
            .x = rect.x, .y = rect.y + rect.h - thickness, .w = rect.w, .h = thickness};

        WidgetUtils::FillRect(batch, topEdge, topLeft);
        WidgetUtils::FillRect(batch, leftEdge, topLeft);
        WidgetUtils::FillRect(batch, rightEdge, bottomRight);
        WidgetUtils::FillRect(batch, bottomEdge, bottomRight);
    }

    void WidgetUtils::DrawBorder(SpriteBatch* batch,
                                 const SDL_Rect& rect,
                                 const BorderColors& colors,
                                 int thickness)
    {
        if (batch == nullptr || thickness <= 0)
        {
            return;
        }

        SDL_Rect topEdge{.x = rect.x, .y = rect.y, .w = rect.w, .h = thickness};
        SDL_Rect leftEdge{.x = rect.x, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_Rect rightEdge{
            .x = rect.x + rect.w - thickness, .y = rect.y, .w = thickness, .h = rect.h};
        SDL_Rect bottomEdge{
            .x = rect.x, .y = rect.y + rect.h - thickness, .w = rect.w, .h = thickness};

        WidgetUtils::FillRect(batch, topEdge, colors.topLeftOuter);
        WidgetUtils::FillRect(batch, leftEdge, colors.topLeftInner);
        WidgetUtils::FillRect(batch, rightEdge, colors.bottomRightInner);
        WidgetUtils::FillRect(batch, bottomEdge, colors.bottomRightOuter);
    }

} // namespace nuvelocity
