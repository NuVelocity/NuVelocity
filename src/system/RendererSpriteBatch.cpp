#include "RendererSpriteBatch.h"
#include "Colors.h"
#include <SDL3/SDL_render.h>

namespace nuvelocity
{
    RendererSpriteBatch::RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window)
            : mRenderer(renderer)
            , mWindow(window)
    {
        mBatchVertices.reserve(1024);
        mBatchIndices.reserve(1536);
    }

    RendererSpriteBatch::~RendererSpriteBatch()
    {
        for (auto& [surface, entry] : mTextureCache)
        {
            if (entry.texture != nullptr)
                SDL_DestroyTexture(entry.texture);
        }
        mTextureCache.clear();
    }

    SDL_Texture* RendererSpriteBatch::GetOrCreateTexture(SDL_Surface* surface)
    {
        if (surface == nullptr)
            return nullptr;

        auto it = mTextureCache.find(surface);
        if (it != mTextureCache.end())
        {
            CachedTexture& entry = it->second;
            if (entry.pixelsSnap != surface->pixels)
            {
                SDL_DestroyTexture(entry.texture);
                entry.texture = nullptr;
            }
            else
            {
                return entry.texture;
            }
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(mRenderer, surface);
        if (tex == nullptr)
            return nullptr;

        mTextureCache[surface] = CachedTexture{tex, surface->pixels};
        return tex;
    }

    void RendererSpriteBatch::InvalidateCache(SDL_Surface* surface)
    {
        auto it = mTextureCache.find(surface);
        if (it != mTextureCache.end())
        {
            if (it->second.texture != nullptr)
                SDL_DestroyTexture(it->second.texture);
            mTextureCache.erase(it);
        }
    }

    static void PushQuad(std::vector<SDL_Vertex>& verts,
                         std::vector<int>& indices,
                         const SDL_FRect& dst,
                         const SDL_FRect* src,
                         float texW,
                         float texH,
                         SDL_Color color)
    {
        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
        if (src != nullptr && texW > 0.0f && texH > 0.0f)
        {
            u0 = src->x / texW;
            v0 = src->y / texH;
            u1 = (src->x + src->w) / texW;
            v1 = (src->y + src->h) / texH;
        }

        const SDL_FColor fc{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};

        const int base = static_cast<int>(verts.size());

        // TL, TR, BR, BL
        verts.push_back({{dst.x, dst.y}, fc, {u0, v0}});
        verts.push_back({{dst.x + dst.w, dst.y}, fc, {u1, v0}});
        verts.push_back({{dst.x + dst.w, dst.y + dst.h}, fc, {u1, v1}});
        verts.push_back({{dst.x, dst.y + dst.h}, fc, {u0, v1}});

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    void RendererSpriteBatch::Draw(SDL_Surface* surface,
                                   const SDL_Rect* destRect,
                                   const SDL_Rect* srcRect,
                                   SDL_Color color)
    {
        if (mRenderer == nullptr || surface == nullptr)
            return;

        SDL_Texture* tex = GetOrCreateTexture(surface);
        if (tex == nullptr)
            return;

        SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
        SDL_GetSurfaceBlendMode(surface, &blendMode);

        if (tex != mBatchTexture || blendMode != mBatchBlendMode)
        {
            Flush();
            mBatchTexture = tex;
            mBatchBlendMode = blendMode;
        }

        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(tex, &texW, &texH);

        const SDL_FRect dst = destRect ? SDL_FRect{static_cast<float>(destRect->x),
                                                   static_cast<float>(destRect->y),
                                                   static_cast<float>(destRect->w),
                                                   static_cast<float>(destRect->h)}
                                       : SDL_FRect{0.0f, 0.0f, texW, texH};

        SDL_FRect src{};
        SDL_FRect* srcPtr = nullptr;
        if (srcRect != nullptr)
        {
            src = {static_cast<float>(srcRect->x),
                   static_cast<float>(srcRect->y),
                   static_cast<float>(srcRect->w),
                   static_cast<float>(srcRect->h)};
            srcPtr = &src;
        }

        PushQuad(mBatchVertices, mBatchIndices, dst, srcPtr, texW, texH, color);

        if (mDrawBounds)
        {
            OutlineRect(destRect, Colors::Magenta);
            if (srcRect != nullptr)
                OutlineRect(srcRect, Colors::Cyan);
        }
    }

    void RendererSpriteBatch::DrawCentered(SDL_Surface* surface)
    {
        if (mRenderer == nullptr || mWindow == nullptr || surface == nullptr)
            return;

        SDL_Texture* tex = GetOrCreateTexture(surface);
        if (tex == nullptr)
            return;

        // Centered draws are uncommon and need a window size query, so bypass batch.
        Flush();

        int winW = 0, winH = 0;
        SDL_GetWindowSizeInPixels(mWindow, &winW, &winH);

        float texW = 0.0f, texH = 0.0f;
        if (!SDL_GetTextureSize(tex, &texW, &texH))
            return;

        const SDL_FRect dst{.x = (static_cast<float>(winW) - texW) * 0.5f,
                            .y = (static_cast<float>(winH) - texH) * 0.5f,
                            .w = texW,
                            .h = texH};

        SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
        SDL_GetSurfaceBlendMode(surface, &blendMode);

        SDL_SetTextureBlendMode(tex, blendMode);
        SDL_RenderTexture(mRenderer, tex, nullptr, &dst);
    }

    void
    RendererSpriteBatch::DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness)
    {
        if (mRenderer == nullptr)
            return;

        Flush();

        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, color.r, color.g, color.b, color.a);
        SDL_RenderLine(mRenderer, x1, y1, x2, y2);
    }

    void RendererSpriteBatch::FillRect(const SDL_Rect* rect, SDL_Color color)
    {
        if (mRenderer == nullptr)
            return;

        Flush();

        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, color.r, color.g, color.b, color.a);

        if (rect != nullptr)
        {
            const SDL_FRect fRect{static_cast<float>(rect->x),
                                  static_cast<float>(rect->y),
                                  static_cast<float>(rect->w),
                                  static_cast<float>(rect->h)};
            SDL_RenderFillRect(mRenderer, &fRect);
        }
        else
        {
            SDL_RenderFillRect(mRenderer, nullptr);
        }
    }

    void RendererSpriteBatch::SetClipRect(const SDL_Rect* rect)
    {
        if (mRenderer == nullptr)
            return;

        Flush();
        SDL_SetRenderClipRect(mRenderer, rect);
    }

    void RendererSpriteBatch::Clear(SDL_Color color)
    {
        if (mRenderer == nullptr)
            return;

        Flush();
        SDL_SetRenderDrawColor(mRenderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(mRenderer);
    }

    void RendererSpriteBatch::Flush()
    {
        if (mBatchVertices.empty() || mBatchTexture == nullptr)
        {
            mBatchVertices.clear();
            mBatchIndices.clear();
            mBatchTexture = nullptr;
            return;
        }

        SDL_SetTextureBlendMode(mBatchTexture, mBatchBlendMode);
        SDL_RenderGeometry(mRenderer,
                           mBatchTexture,
                           mBatchVertices.data(),
                           static_cast<int>(mBatchVertices.size()),
                           mBatchIndices.data(),
                           static_cast<int>(mBatchIndices.size()));

        mBatchVertices.clear();
        mBatchIndices.clear();
        mBatchTexture = nullptr;
    }

    void RendererSpriteBatch::Present()
    {
        if (mRenderer != nullptr)
        {
            Flush();
            SDL_RenderPresent(mRenderer);
        }
    }

} // namespace nuvelocity
