#ifndef NVE_GPU_SPRITE_BATCH_H
#define NVE_GPU_SPRITE_BATCH_H

#include <SDL3/SDL.h>

#include "SpriteBatch.h"

namespace nuvelocity
{
    class Image;

    class GPUSpriteBatch : public SpriteBatch
    {
    public:
        explicit GPUSpriteBatch(SDL_GPUDevice* device);
        GPUSpriteBatch(SDL_GPUDevice* device, SDL_Window* window);
        ~GPUSpriteBatch() override;

        GPUSpriteBatch(const GPUSpriteBatch&) = delete;
        GPUSpriteBatch& operator=(const GPUSpriteBatch&) = delete;

        // Render an image with optional dest/src rects
        void DrawImage(Image& image,
                       const SDL_FRect* destRect = nullptr,
                       const SDL_FRect* srcRect = nullptr) override;

        // Render an image centered on the window
        void DrawImageCentered(Image& image) override;

        // Submit the current command buffer and flush all pending draws
        void Flush() override;

    private:
        SDL_GPUDevice* mDevice;
        SDL_Window* mWindow;
        SDL_GPUCommandBuffer* mCommandBuffer;
        SDL_GPUTexture* mSwapchainTexture;
        Uint32 mSwapchainWidth;
        Uint32 mSwapchainHeight;
        bool mHasBlittedThisFrame;

        void EnsureCommandBuffer();
        bool EnsureSwapchainTexture();
        SDL_GPUTexture* CreateAndUploadTexture(SDL_Surface* surface);
        void SubmitCommandBuffer();
    };
} // namespace nuvelocity

#endif // NVE_GPU_SPRITE_BATCH_H
