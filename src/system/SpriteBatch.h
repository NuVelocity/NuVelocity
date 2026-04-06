#ifndef NVE_SPRITE_BATCH_H
#define NVE_SPRITE_BATCH_H

#include <SDL3/SDL.h>

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

        virtual void DrawCentered(SDL_Surface* surface) = 0;

        virtual void DrawLine(
            float x1, float y1, float x2, float y2, SDL_Color color, float thickness = 1.0f) = 0;

        virtual void FillRect(const SDL_FRect* rect, SDL_Color color) = 0;

        virtual void SetClipRect(const SDL_Rect* rect) = 0;

        virtual void Clear(SDL_Color color) = 0;

        virtual void Flush() = 0;

        virtual void Present() = 0;

    protected:
        SpriteBatch() = default;
    };
} // namespace nuvelocity

#endif // NVE_SPRITE_BATCH_H
