#ifndef NVE_SPRITE_BATCH_H
#define NVE_SPRITE_BATCH_H

#include "API.h"
#include <SDL3/SDL.h>

#include "Colors.h"
#include <cstddef>

namespace nuvelocity
{
    class Sequence;
    class StandAloneFrame;

    // Common sprite batch interface implemented by a graphics backend.
    class SpriteBatch
    {
    public:
        NVE_API virtual ~SpriteBatch() = default;

        NVE_API SpriteBatch(const SpriteBatch&) = delete;
        NVE_API SpriteBatch& operator=(const SpriteBatch&) = delete;

        NVE_API SpriteBatch(SpriteBatch&&) = delete;
        NVE_API SpriteBatch& operator=(SpriteBatch&&) = delete;

        NVE_API virtual void Draw(SDL_Surface* surface,
                                  const SDL_Rect* destRect = nullptr,
                                  const SDL_Rect* srcRect = nullptr,
                                  SDL_Color color = Colors::White) = 0;

        NVE_API virtual void Draw(StandAloneFrame* frame,
                                  int x,
                                  int y,
                                  SDL_Color color = Colors::White,
                                  bool useHotSpot = true);

        NVE_API virtual void Draw(Sequence* sequence,
                                  std::size_t frameIndex,
                                  int x,
                                  int y,
                                  SDL_Color color = Colors::White,
                                  bool useHotSpot = true);

        NVE_API virtual void DrawCentered(SDL_Surface* surface) = 0;

        NVE_API virtual void DrawCentered(StandAloneFrame* frame);

        NVE_API virtual void DrawCentered(Sequence* sequence, std::size_t frameIndex);

        NVE_API virtual void
        DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) = 0;

        NVE_API virtual void OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness = 1);

        NVE_API virtual void FillRect(const SDL_Rect* rect, SDL_Color color) = 0;

        NVE_API void SetDrawBounds(bool enabled)
        {
            mDrawBounds = enabled;
        }

        NVE_API bool IsDrawBoundsEnabled() const
        {
            return mDrawBounds;
        }

        NVE_API virtual void SetClipRect(const SDL_Rect* rect) = 0;

        NVE_API virtual void Clear(SDL_Color color) = 0;

        NVE_API virtual void Flush() = 0;

        NVE_API virtual void Present() = 0;

    protected:
        SpriteBatch() = default;

        bool mDrawBounds = false;
    };
} // namespace nuvelocity

#endif // NVE_SPRITE_BATCH_H
