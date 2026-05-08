#include "RendererSpriteBatch.h"
#include "Colors.h"
#include <SDL3/SDL_render.h>
#include <algorithm>
#include <cmath>

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

    static void PushQuadV(std::vector<SDL_Vertex>& verts,
                          std::vector<int>& indices,
                          SDL_FPoint p0,
                          SDL_FPoint p1,
                          SDL_FPoint p2,
                          SDL_FPoint p3,
                          SDL_FColor c0,
                          SDL_FColor c1,
                          SDL_FColor c2,
                          SDL_FColor c3)
    {
        const int base = static_cast<int>(verts.size());

        verts.push_back({p0, c0, {0.0f, 0.0f}});
        verts.push_back({p1, c1, {1.0f, 0.0f}});
        verts.push_back({p2, c2, {1.0f, 1.0f}});
        verts.push_back({p3, c3, {0.0f, 1.0f}});

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    void RendererSpriteBatch::DrawLine(
        float x1, float y1, float x2, float y2, SDL_Color color, float thickness)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        Flush();

        const float fx1 = x1;
        const float fy1 = y1;
        const float fx2 = x2;
        const float fy2 = y2;
        const float fThickness = thickness;

        const float dx = fx2 - fx1;
        const float dy = fy2 - fy1;
        const float len = std::sqrt((dx * dx) + (dy * dy));
        if (len < 0.001F)
        {
            return;
        }

        const SDL_FColor cFull = {
            color.r / 255.0F, color.g / 255.0F, color.b / 255.0F, color.a / 255.0F};

        const bool isStraight = (std::abs(dx) < 0.001F || std::abs(dy) < 0.001F);
        const bool isInteger = (std::fmod(fx1, 1.0F) == 0.0F && std::fmod(fy1, 1.0F) == 0.0F &&
                                std::fmod(fx2, 1.0F) == 0.0F && std::fmod(fy2, 1.0F) == 0.0F &&
                                std::fmod(fThickness, 1.0F) == 0.0F);

        if (isStraight && isInteger)
        {
            float wx = 0.0F;
            float wy = 0.0F;
            float ox = 0.0F;
            float oy = 0.0F;
            if (std::abs(dx) < 0.001F)
            {
                wx = fThickness * 0.5F;
                if (static_cast<int>(fThickness) % 2 != 0)
                {
                    ox = 0.5F;
                }
            }
            else
            {
                wy = fThickness * 0.5F;
                if (static_cast<int>(fThickness) % 2 != 0)
                {
                    oy = 0.5F;
                }
            }

            std::vector<SDL_Vertex> verts;
            std::vector<int> indices;
            PushQuadV(verts,
                      indices,
                      {fx1 + wx + ox, fy1 + wy + oy},
                      {fx1 - wx + ox, fy1 - wy + oy},
                      {fx2 - wx + ox, fy2 - wy + oy},
                      {fx2 + wx + ox, fy2 + wy + oy},
                      cFull,
                      cFull,
                      cFull,
                      cFull);

            SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
            SDL_RenderGeometry(mRenderer,
                               nullptr,
                               verts.data(),
                               static_cast<int>(verts.size()),
                               indices.data(),
                               static_cast<int>(indices.size()));
            return;
        }

        const float nx = -dy / len;
        const float ny = dx / len;

        const SDL_FColor cFade = {color.r / 255.0F, color.g / 255.0F, color.b / 255.0F, 0.0F};

        const float halfCore = std::max(0.0F, (fThickness - 1.0F) * 0.5F);
        const float halfTotal = halfCore + 1.0F;

        std::vector<SDL_Vertex> verts;
        std::vector<int> indices;
        verts.reserve(12);
        indices.reserve(18);

        // Core
        if (halfCore > 0.0F)
        {
            PushQuadV(verts,
                      indices,
                      {fx1 + nx * halfCore, fy1 + ny * halfCore},
                      {fx1 - nx * halfCore, fy1 - ny * halfCore},
                      {fx2 - nx * halfCore, fy2 - ny * halfCore},
                      {fx2 + nx * halfCore, fy2 + ny * halfCore},
                      cFull,
                      cFull,
                      cFull,
                      cFull);
        }

        // Top fringe
        PushQuadV(verts,
                  indices,
                  {fx1 + nx * halfTotal, fy1 + ny * halfTotal},
                  {fx1 + nx * halfCore, fy1 + ny * halfCore},
                  {fx2 + nx * halfCore, fy2 + ny * halfCore},
                  {fx2 + nx * halfTotal, fy2 + ny * halfTotal},
                  cFade,
                  cFull,
                  cFull,
                  cFade);

        // Bottom fringe
        PushQuadV(verts,
                  indices,
                  {fx1 - nx * halfCore, fy1 - ny * halfCore},
                  {fx1 - nx * halfTotal, fy1 - ny * halfTotal},
                  {fx2 - nx * halfTotal, fy2 - ny * halfTotal},
                  {fx2 - nx * halfCore, fy2 - ny * halfCore},
                  cFull,
                  cFade,
                  cFade,
                  cFull);

        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(mRenderer,
                           nullptr,
                           verts.data(),
                           static_cast<int>(verts.size()),
                           indices.data(),
                           static_cast<int>(indices.size()));
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
