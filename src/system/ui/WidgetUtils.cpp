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

        SDL_Rect srcRect{.x = 0, .y = 0, .w = surface->w, .h = surface->h};
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

        SDL_Rect srcRect{.x = 0, .y = 0, .w = surface->w, .h = surface->h};
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

        SDL_Rect srcRect{.x = 0, .y = 0, .w = surface->w, .h = surface->h};
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
                                const BorderColors& colors,
                                int thickness,
                                int sides)
    {
        if (batch == nullptr || thickness <= 0)
        {
            return;
        }

        // Draw a layered bevel by drawing thickness inset lines (rings).
        // For each inset i (0..thickness-1) draw the top/left and bottom/right
        // edges inset by i pixels.
        for (int i = 0; i < thickness; ++i)
        {
            // Choose colors: outer half uses the "outer" colors, inner half uses "inner".
            const SDL_Color topLeftColor =
                (i < (thickness / 2)) ? colors.topLeftOuter : colors.topLeftInner;
            const SDL_Color bottomRightColor =
                (i < (thickness / 2)) ? colors.bottomRightOuter : colors.bottomRightInner;

            const int insetX = rect.x + i;
            const int insetY = rect.y + i;
            const int w = rect.w - 2 * i;
            const int h = rect.h - 2 * i;

            if (w <= 0 || h <= 0)
            {
                break;
            }

            if (sides & Side_Top)
            {
                SDL_Rect topLine{.x = insetX, .y = insetY, .w = w, .h = 1};
                WidgetUtils::FillRect(batch, topLine, topLeftColor);
            }

            if (sides & Side_Left)
            {
                SDL_Rect leftLine{.x = insetX, .y = insetY, .w = 1, .h = h};
                WidgetUtils::FillRect(batch, leftLine, topLeftColor);
            }

            if (sides & Side_Right)
            {
                SDL_Rect rightLine{.x = insetX + w - 1, .y = insetY, .w = 1, .h = h};
                WidgetUtils::FillRect(batch, rightLine, bottomRightColor);
            }

            if (sides & Side_Bottom)
            {
                SDL_Rect bottomLine{.x = insetX, .y = insetY + h - 1, .w = w, .h = 1};
                WidgetUtils::FillRect(batch, bottomLine, bottomRightColor);
            }
        }
    }

    void WidgetUtils::DrawBorder(SpriteBatch* batch,
                                 const SDL_Rect& rect,
                                 const BorderColors& colors,
                                 int thickness,
                                 int sides)
    {
        if (batch == nullptr || thickness <= 0)
        {
            return;
        }

        if (sides & Side_Top)
        {
            SDL_Rect topEdge{.x = rect.x, .y = rect.y, .w = rect.w, .h = thickness};
            WidgetUtils::FillRect(batch, topEdge, colors.topLeftOuter);
        }
        if (sides & Side_Left)
        {
            SDL_Rect leftEdge{.x = rect.x, .y = rect.y, .w = thickness, .h = rect.h};
            WidgetUtils::FillRect(batch, leftEdge, colors.topLeftInner);
        }
        if (sides & Side_Right)
        {
            SDL_Rect rightEdge{
                .x = rect.x + rect.w - thickness, .y = rect.y, .w = thickness, .h = rect.h};
            WidgetUtils::FillRect(batch, rightEdge, colors.bottomRightInner);
        }
        if (sides & Side_Bottom)
        {
            SDL_Rect bottomEdge{
                .x = rect.x, .y = rect.y + rect.h - thickness, .w = rect.w, .h = thickness};
            WidgetUtils::FillRect(batch, bottomEdge, colors.bottomRightOuter);
        }
    }

    void WidgetUtils::DrawBevel(SpriteBatch* batch,
                                const SDL_Rect& rect,
                                const BevelColors& colors,
                                bool sunken,
                                int thickness,
                                int sides)
    {
        BorderColors bc;
        if (sunken)
        {
            bc.topLeftOuter = colors.dark;
            bc.topLeftInner = colors.dark;
            bc.bottomRightInner = colors.light;
            bc.bottomRightOuter = colors.light;
        }
        else
        {
            bc.topLeftOuter = colors.light;
            bc.topLeftInner = colors.light;
            bc.bottomRightInner = colors.dark;
            bc.bottomRightOuter = colors.dark;
        }
        DrawBevel(batch, rect, bc, thickness, sides);
    }
} // namespace nuvelocity
