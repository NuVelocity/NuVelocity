#ifndef NVE_SPRITE_BATCH_H
#define NVE_SPRITE_BATCH_H

#include <SDL3/SDL.h>

namespace nuvelocity
{
    class Image;

    // Common sprite batch interface implemented by a graphics backend.
    class SpriteBatch
    {
    public:
        virtual ~SpriteBatch() = default;

        SpriteBatch(const SpriteBatch&) = delete;
        SpriteBatch& operator=(const SpriteBatch&) = delete;

        SpriteBatch(SpriteBatch&&) = delete;
        SpriteBatch& operator=(SpriteBatch&&) = delete;

        virtual void DrawImage(Image& image,
                               const SDL_FRect* destRect = nullptr,
                               const SDL_FRect* srcRect = nullptr) = 0;

        virtual void DrawImageCentered(Image& image) = 0;

        virtual void Flush() = 0;

    protected:
        SpriteBatch() = default;
    };
} // namespace nuvelocity

#endif // NVE_SPRITE_BATCH_H
