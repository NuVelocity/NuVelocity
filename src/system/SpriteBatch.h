#ifndef NVE_SPRITE_BATCH_H
#define NVE_SPRITE_BATCH_H

#include <SDL3/SDL.h>

#include "Sequence.h"
#include "StandAloneFrame.h"

namespace nuvelocity
{

    // Common sprite batch interface implemented by a graphics backend.
    class SpriteBatch
    {
    public:
        virtual ~SpriteBatch() = default;

        SpriteBatch(const SpriteBatch&) = delete;
        SpriteBatch& operator=(const SpriteBatch&) = delete;

        SpriteBatch(SpriteBatch&&) = delete;
        SpriteBatch& operator=(SpriteBatch&&) = delete;

        virtual void Draw(SDL_Surface* surface,
                          const SDL_Rect* destRect = nullptr,
                          const SDL_Rect* srcRect = nullptr,
                          SDL_Color color = {255, 255, 255, 255}) = 0;

        virtual void Draw(StandAloneFrame* frame,
                          int x,
                          int y,
                          SDL_Color color = {255, 255, 255, 255},
                          bool useHotSpot = true)
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

            SDL_Rect destRect{.x = x + (useHotSpot ? frame->mHotSpot.x + (surface->w / 2) : 0),
                              .y = y + (useHotSpot ? frame->mHotSpot.y + (surface->h / 2) : 0),
                              .w = surface->w,
                              .h = surface->h};

            Draw(surface, &destRect, nullptr, color);
        }

        virtual void Draw(Sequence* sequence,
                          std::size_t frameIndex,
                          int x,
                          int y,
                          SDL_Color color = {255, 255, 255, 255},
                          bool useHotSpot = true)
        {
            if (sequence == nullptr || frameIndex >= sequence->GetFrameCount())
            {
                return;
            }

            Frame* frame = sequence->GetFrame(frameIndex);
            if (frame == nullptr)
            {
                return;
            }

            SDL_Surface* surface = frame->GetSurface();
            if (surface == nullptr)
            {
                return;
            }

            SDL_Rect destRect{.x = x + (useHotSpot ? frame->mHotSpot.x : 0),
                              .y = y + (useHotSpot ? frame->mHotSpot.y : 0),
                              .w = surface->w,
                              .h = surface->h};
            Draw(surface, &destRect, nullptr, color);
        }

        virtual void DrawCentered(SDL_Surface* surface) = 0;

        virtual void DrawCentered(StandAloneFrame* frame)
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

            DrawCentered(surface);
        }

        virtual void DrawCentered(Sequence* sequence, std::size_t frameIndex)
        {
            if (sequence == nullptr || frameIndex >= sequence->GetFrameCount())
            {
                return;
            }

            SDL_Surface* surface = sequence->GetSurface(frameIndex);
            if (surface == nullptr)
            {
                return;
            }

            DrawCentered(surface);
        }

        virtual void DrawLine(
            int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) = 0;

        virtual void OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness = 1)
        {
            DrawLine(rect->x, rect->y, rect->x + rect->w, rect->y, color, thickness);
            DrawLine(
                rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h, color, thickness);
            DrawLine(
                rect->x + rect->w, rect->y + rect->h, rect->x, rect->y + rect->h, color, thickness);
            DrawLine(rect->x, rect->y + rect->h, rect->x, rect->y, color, thickness);
        }

        virtual void FillRect(const SDL_Rect* rect, SDL_Color color) = 0;

        void SetDrawBounds(bool enabled)
        {
            mDrawBounds = enabled;
        }

        bool IsDrawBoundsEnabled() const
        {
            return mDrawBounds;
        }

        virtual void SetClipRect(const SDL_Rect* rect) = 0;

        virtual void Clear(SDL_Color color) = 0;

        virtual void Flush() = 0;

        virtual void Present() = 0;

    protected:
        SpriteBatch() = default;

        bool mDrawBounds = false;
    };
} // namespace nuvelocity

#endif // NVE_SPRITE_BATCH_H
