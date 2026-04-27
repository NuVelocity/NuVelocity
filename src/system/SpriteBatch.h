#ifndef NVE_SPRITE_BATCH_H
#define NVE_SPRITE_BATCH_H

#include <SDL3/SDL.h>

#include "Colors.h"
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
                          SDL_Color color = Colors::White) = 0;

        virtual void Draw(StandAloneFrame* frame,
                          int x,
                          int y,
                          SDL_Color color = Colors::White,
                          bool useHotSpot = true);

        virtual void Draw(Sequence* sequence,
                          std::size_t frameIndex,
                          int x,
                          int y,
                          SDL_Color color = Colors::White,
                          bool useHotSpot = true);

        virtual void DrawCentered(SDL_Surface* surface) = 0;

        virtual void DrawCentered(StandAloneFrame* frame);

        virtual void DrawCentered(Sequence* sequence, std::size_t frameIndex);

        virtual void
        DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) = 0;

        virtual void OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness = 1);

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
