#ifndef NVE_RENDERER_SPRITE_BATCH_H
#define NVE_RENDERER_SPRITE_BATCH_H

#include "Colors.h"
#include "SpriteBatch.h"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{

    // SDL Renderer-based sprite batch implementation
    class RendererSpriteBatch : public SpriteBatch
    {
    public:
        RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window);
        ~RendererSpriteBatch() override;

        RendererSpriteBatch(const RendererSpriteBatch&) = delete;
        RendererSpriteBatch& operator=(const RendererSpriteBatch&) = delete;

        void Draw(SDL_Surface* surface,
                  const SDL_Rect* destRect = nullptr,
                  const SDL_Rect* srcRect = nullptr,
                  SDL_Color color = Colors::White) override;

        void DrawCentered(SDL_Surface* surface) override;

        void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) override;

        void FillRect(const SDL_Rect* rect, SDL_Color color) override;

        void SetClipRect(const SDL_Rect* rect) override;

        void Clear(SDL_Color color) override;

        void Flush() override;

        void Present() override;

        // Call when a surface's pixels have been modified so the cached
        // GPU texture gets rebuilt on the next draw.
        void InvalidateCache(SDL_Surface* surface);

    private:
        // Cached texture entry. Stores surface->pixels at upload time so we can
        // detect in-place surface modifications and rebuild automatically.
        struct CachedTexture
        {
            SDL_Texture* texture = nullptr;
            void* pixelsSnap = nullptr;
        };

        // Returns (lazily creating) the GPU texture for a surface.
        // Rebuilds automatically if surface->pixels has changed since last upload.
        SDL_Texture* GetOrCreateTexture(SDL_Surface* surface);

        SDL_Renderer* mRenderer = nullptr;
        SDL_Window* mWindow = nullptr;

        // Texture cache
        std::unordered_map<SDL_Surface*, CachedTexture> mTextureCache;

        // Consecutive sprite draws sharing the same texture + blend mode are
        // accumulated here and submitted as one SDL_RenderGeometry call.
        SDL_Texture* mBatchTexture = nullptr;
        SDL_BlendMode mBatchBlendMode = SDL_BLENDMODE_BLEND;
        std::vector<SDL_Vertex> mBatchVertices;
        std::vector<int> mBatchIndices;
    };

} // namespace nuvelocity

#endif // NVE_RENDERER_SPRITE_BATCH_H
