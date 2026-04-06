#ifndef NVE_GPU_SPRITE_BATCH_H
#define NVE_GPU_SPRITE_BATCH_H

#include <SDL3/SDL.h>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "SpriteBatch.h"

namespace nuvelocity
{
    struct SpriteVertex
    {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    class GPUSpriteBatch : public SpriteBatch
    {
    public:
        explicit GPUSpriteBatch(SDL_GPUDevice* device);
        GPUSpriteBatch(SDL_GPUDevice* device, SDL_Window* window);
        ~GPUSpriteBatch() override;

        GPUSpriteBatch(const GPUSpriteBatch&) = delete;
        GPUSpriteBatch& operator=(const GPUSpriteBatch&) = delete;

        void Draw(SDL_Surface* surface,
                  const SDL_FRect* destRect = nullptr,
                  const SDL_FRect* srcRect = nullptr,
                  SDL_Color color = {255, 255, 255, 255}) override;

        void DrawCentered(SDL_Surface* surface) override;

        void DrawLine(float x1,
                      float y1,
                      float x2,
                      float y2,
                      SDL_Color color,
                      float thickness = 1.0f) override;

        void FillRect(const SDL_FRect* rect, SDL_Color color) override;

        void SetClipRect(const SDL_Rect* rect) override;

        void Clear(SDL_Color color) override;

        void Flush() override;

        void Present() override;

    private:
        SDL_GPUDevice* mDevice;
        SDL_Window* mWindow;
        SDL_GPUCommandBuffer* mCommandBuffer;
        SDL_GPUTexture* mSwapchainTexture;
        Uint32 mSwapchainWidth;
        Uint32 mSwapchainHeight;
        bool mHasBlittedThisFrame;
        SDL_FColor mClearColor{.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F};
        bool mNeedsClear{false};

        SDL_GPUGraphicsPipeline* mPipeline;
        SDL_GPUSampler* mSampler;
        SDL_GPUTexture* mWhiteTexture;

        std::vector<SpriteVertex> mVertexData;
        std::vector<Uint16> mIndexData;
        SDL_GPUTexture* mCurrentTexture;
        SDL_Rect mCurrentClipRect;
        bool mHasCurrentClipRect;
        std::unordered_map<SDL_Surface*, SDL_GPUTexture*> mTextureCache;

        void EnsureCommandBuffer();
        bool EnsureSwapchainTexture();
        SDL_GPUTexture* CreateAndUploadTexture(SDL_Surface* surface);
        void SubmitCommandBuffer();
        void FlushBatch();
        void InitializePipeline();
    };
} // namespace nuvelocity

#endif // NVE_GPU_SPRITE_BATCH_H
