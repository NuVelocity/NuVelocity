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
                          const SDL_FRect* destRect = nullptr,
                          const SDL_FRect* srcRect = nullptr,
                          SDL_Color color = {255, 255, 255, 255}) = 0;

        virtual void
        Draw(StandAloneFrame* frame, float x, float y, SDL_Color color = {255, 255, 255, 255})
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

            SDL_FRect destRect{
                x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};

            Draw(surface, &destRect, nullptr, color);
        }

        virtual void Draw(Sequence* sequence,
                          std::size_t frameIndex,
                          float x,
                          float y,
                          SDL_Color color = {255, 255, 255, 255})
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

            SDL_FRect destRect{
                x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
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
            float x1, float y1, float x2, float y2, SDL_Color color, float thickness = 1.0f) = 0;

        virtual void OutlineRect(const SDL_FRect* rect, SDL_Color color, float thickness = 1.0f)
        {
            DrawLine(rect->x, rect->y, rect->x + rect->w, rect->y, color, thickness);
            DrawLine(
                rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h, color, thickness);
            DrawLine(
                rect->x + rect->w, rect->y + rect->h, rect->x, rect->y + rect->h, color, thickness);
            DrawLine(rect->x, rect->y + rect->h, rect->x, rect->y, color, thickness);
        }

        virtual void FillRect(const SDL_FRect* rect, SDL_Color color) = 0;

        void SetDebugDrawing(bool enabled)
        {
            mDebugDrawing = enabled;
        }

        bool IsDebugDrawingEnabled() const
        {
            return mDebugDrawing;
        }

        virtual void SetClipRect(const SDL_Rect* rect) = 0;

        virtual void Clear(SDL_Color color) = 0;

        virtual void Flush() = 0;

        virtual void Present() = 0;

    protected:
        SpriteBatch() = default;

        bool mDebugDrawing = false;
    };
} // namespace nuvelocity

#endif // NVE_SPRITE_BATCH_H
