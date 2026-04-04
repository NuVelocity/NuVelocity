#ifndef NVE_RENDERER_SPRITE_BATCH_H
#define NVE_RENDERER_SPRITE_BATCH_H

#include <SDL3/SDL.h>
#include <vector>

#include "SpriteBatch.h"

namespace nuvelocity
{
    class Image;

    // SDL Renderer-based sprite batch implementation
    class RendererSpriteBatch : public SpriteBatch
    {
    public:
        RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window);
        ~RendererSpriteBatch() override = default;

        RendererSpriteBatch(const RendererSpriteBatch&) = delete;
        RendererSpriteBatch& operator=(const RendererSpriteBatch&) = delete;

        void DrawImage(Image& image,
                       const SDL_FRect* destRect = nullptr,
                       const SDL_FRect* srcRect = nullptr) override;

        void DrawImageCentered(Image& image) override;

        void Flush() override;

    private:
        struct DrawCommand
        {
            Image* image;
            bool centered;
            bool hasDestRect;
            SDL_FRect destRect;
            bool hasSrcRect;
            SDL_FRect srcRect;
        };

        SDL_Renderer* mRenderer;
        SDL_Window* mWindow;
        std::vector<DrawCommand> mDrawCommands;
    };
} // namespace nuvelocity

#endif // NVE_RENDERER_SPRITE_BATCH_H
