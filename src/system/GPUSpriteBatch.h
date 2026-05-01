#ifndef NVE_GPU_SPRITE_BATCH_H
#define NVE_GPU_SPRITE_BATCH_H

#include <SDL3/SDL.h>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Colors.h"
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
                  const SDL_Rect* destRect = nullptr,
                  const SDL_Rect* srcRect = nullptr,
                  SDL_Color color = Colors::White) override;

        void DrawCentered(SDL_Surface* surface) override;

        void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) override;

        void FillRect(const SDL_Rect* rect, SDL_Color color) override;

        void OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness = 1) override;

        void SetClipRect(const SDL_Rect* rect) override;

        void Clear(SDL_Color color) override;

        void Flush() override;

        void Present() override;

    private:
        // Texture cache entry. This stores the GPU texture together with a snapshot of
        // the surface's pixel pointer so we can detect stale entries when a surface
        // is destroyed and a new one happens to be allocated at the same address.
        struct CachedTexture
        {
            SDL_GPUTexture* texture = nullptr;
            const void* pixelsSnapshot = nullptr; // surface->pixels at upload time
        };

        SDL_GPUDevice* mDevice;
        SDL_Window* mWindow;
        SDL_GPUCommandBuffer* mCommandBuffer;
        SDL_GPUTexture* mSwapchainTexture;
        Uint32 mSwapchainWidth;
        Uint32 mSwapchainHeight;
        bool mHasBlittedThisFrame;

        SDL_FColor mClearColor{.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F};
        bool mNeedsClear{false};

        std::unordered_map<SDL_BlendMode, SDL_GPUGraphicsPipeline*> mPipelines;
        SDL_GPUSampler* mSampler = nullptr;
        SDL_GPUTexture* mWhiteTexture = nullptr;

        // Per-frame CPU-side geometry staging
        std::vector<SpriteVertex> mVertexData;
        std::vector<Uint16> mIndexData;

        SDL_GPUTexture* mCurrentTexture;
        SDL_BlendMode mCurrentBlendMode;
        SDL_Rect mCurrentClipRect{};
        bool mHasCurrentClipRect;

        // Texture cache
        std::unordered_map<SDL_Surface*, CachedTexture> mTextureCache;

        // Persistent GPU buffers — allocated once, grown on demand, never destroyed
        // until the batch itself is destroyed.
        SDL_GPUBuffer* mVertexGPUBuffer;
        SDL_GPUBuffer* mIndexGPUBuffer;
        Uint32 mVertexGPUBufferCapacity;
        Uint32 mIndexGPUBufferCapacity;

        void EnsureCommandBuffer();
        bool EnsureSwapchainTexture();

        SDL_GPUTexture* CreateAndUploadTexture(SDL_Surface* surface);

        void EnsureVertexBufferCapacity(Uint32 neededBytes);
        void EnsureIndexBufferCapacity(Uint32 neededBytes);

        // Appends one quad (4 vertices + 6 indices) to the staging vectors,
        // auto-flushing first if a Uint16 index overflow would occur.
        void PushQuad(float x0,
                      float y0,
                      float x1,
                      float y1,
                      float x2,
                      float y2,
                      float x3,
                      float y3,
                      float u0,
                      float v0,
                      float u1,
                      float v1,
                      float r,
                      float g,
                      float b,
                      float a);

        void SubmitCommandBuffer();
        void FlushBatch();
        void InitializePipelines();
        SDL_GPUGraphicsPipeline* CreatePipelineForBlendMode(SDL_BlendMode mode);
    };

} // namespace nuvelocity

#endif // NVE_GPU_SPRITE_BATCH_H
