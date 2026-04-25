#include "RendererSpriteBatch.h"
#include <SDL3/SDL_render.h>

namespace nuvelocity
{
    constexpr float kCenterFactor = 0.5F;

    RendererSpriteBatch::RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window)
            : mRenderer(renderer)
            , mWindow(window)
    {
    }

    RendererSpriteBatch::~RendererSpriteBatch()
    {
        for (auto& [surface, texture] : mTextureCache)
        {
            if (texture != nullptr)
            {
                SDL_DestroyTexture(texture);
            }
        }
        mTextureCache.clear();
    }

    void RendererSpriteBatch::Draw(SDL_Surface* surface,
                                   const SDL_Rect* destRect,
                                   const SDL_Rect* srcRect,
                                   SDL_Color color)
    {
        if (mRenderer == nullptr || surface == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::DrawSurface,
                            .surface = surface,
                            .texture = nullptr,
                            .centered = false,
                            .hasDestRect = destRect != nullptr,
                            .destRect = destRect != nullptr ? *destRect : SDL_Rect{},
                            .hasSrcRect = srcRect != nullptr,
                            .srcRect = srcRect != nullptr ? *srcRect : SDL_Rect{},
                            .color = color,
                            .blendMode = SDL_BLENDMODE_BLEND};
        if (surface != nullptr)
        {
            SDL_GetSurfaceBlendMode(surface, &command.blendMode);
        }
        mDrawCommands.push_back(command);

        if (mDrawBounds)
        {
            OutlineRect(destRect, {255, 0, 255, 255}); // Magenta: Actual
            if (srcRect != nullptr)
            {
                OutlineRect(srcRect, {0, 255, 255, 255}); // Cyan: Source
            }
        }
    }

    void RendererSpriteBatch::DrawCentered(SDL_Surface* surface)
    {
        if (mRenderer == nullptr || mWindow == nullptr || surface == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::DrawSurface,
                            .surface = surface,
                            .texture = nullptr,
                            .centered = true,
                            .hasDestRect = false,
                            .destRect = SDL_Rect{},
                            .hasSrcRect = false,
                            .srcRect = SDL_Rect{},
                            .color = {255, 255, 255, 255},
                            .blendMode = SDL_BLENDMODE_BLEND};
        if (surface != nullptr)
        {
            SDL_GetSurfaceBlendMode(surface, &command.blendMode);
        }
        mDrawCommands.push_back(command);
    }

    void
    RendererSpriteBatch::DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::DrawLine,
                            .color = color,
                            .x1 = x1,
                            .y1 = y1,
                            .x2 = x2,
                            .y2 = y2,
                            .thickness = thickness,
                            .blendMode = SDL_BLENDMODE_BLEND};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::FillRect(const SDL_Rect* rect, SDL_Color color)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::FillRect,
                            .hasDestRect = rect != nullptr,
                            .destRect = rect != nullptr ? *rect : SDL_Rect{},
                            .color = color};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::SetClipRect(const SDL_Rect* rect)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::SetClipRect,
                            .hasClipRect = rect != nullptr,
                            .clipRect = rect != nullptr ? *rect : SDL_Rect{}};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::Clear(SDL_Color color)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::Clear, .color = color};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::Flush()
    {
        if (mRenderer == nullptr)
        {
            mDrawCommands.clear();
            return;
        }

        for (DrawCommand& command : mDrawCommands)
        {
            switch (command.type)
            {
            case CommandType::DrawSurface:
            {
                if (command.surface == nullptr)
                    continue;

                auto cacheIt = mTextureCache.find(command.surface);
                SDL_Texture* texture = (cacheIt != mTextureCache.end()) ? cacheIt->second : nullptr;

                if (texture == nullptr)
                {
                    texture = SDL_CreateTextureFromSurface(mRenderer, command.surface);
                    if (texture == nullptr)
                        continue;
                    mTextureCache[command.surface] = texture;
                }

                SDL_SetTextureColorMod(texture, command.color.r, command.color.g, command.color.b);
                SDL_SetTextureAlphaMod(texture, command.color.a);
                SDL_SetTextureBlendMode(texture, command.blendMode);

                if (command.centered)
                {
                    if (mWindow == nullptr)
                    {
                        SDL_DestroyTexture(texture);
                        continue;
                    }

                    int winWidth = 0;
                    int winHeight = 0;
                    SDL_GetWindowSizeInPixels(mWindow, &winWidth, &winHeight);

                    float texWidth = 0.0F;
                    float texHeight = 0.0F;
                    if (!SDL_GetTextureSize(texture, &texWidth, &texHeight))
                    {
                        SDL_DestroyTexture(texture);
                        continue;
                    }

                    SDL_Rect centeredRect{.x = (winWidth - static_cast<int>(texWidth)) / 2,
                                          .y = (winHeight - static_cast<int>(texHeight)) / 2,
                                          .w = static_cast<int>(texWidth),
                                          .h = static_cast<int>(texHeight)};

                    const SDL_FRect fCenteredRect{static_cast<float>(centeredRect.x),
                                                  static_cast<float>(centeredRect.y),
                                                  static_cast<float>(centeredRect.w),
                                                  static_cast<float>(centeredRect.h)};

                    SDL_RenderTexture(mRenderer, texture, nullptr, &fCenteredRect);
                }
                else
                {
                    const SDL_FRect fSrc{static_cast<float>(command.srcRect.x),
                                         static_cast<float>(command.srcRect.y),
                                         static_cast<float>(command.srcRect.w),
                                         static_cast<float>(command.srcRect.h)};
                    const SDL_FRect fDst{static_cast<float>(command.destRect.x),
                                         static_cast<float>(command.destRect.y),
                                         static_cast<float>(command.destRect.w),
                                         static_cast<float>(command.destRect.h)};

                    const SDL_FRect* src = command.hasSrcRect ? &fSrc : nullptr;
                    const SDL_FRect* dst = command.hasDestRect ? &fDst : nullptr;
                    SDL_RenderTexture(mRenderer, texture, src, dst);
                }
                break;
            }
            case CommandType::DrawTexture:
            {
                if (command.texture == nullptr)
                    continue;

                SDL_SetTextureColorMod(
                    command.texture, command.color.r, command.color.g, command.color.b);
                SDL_SetTextureAlphaMod(command.texture, command.color.a);
                SDL_SetTextureBlendMode(command.texture, command.blendMode);

                const SDL_FRect fSrc{static_cast<float>(command.srcRect.x),
                                     static_cast<float>(command.srcRect.y),
                                     static_cast<float>(command.srcRect.w),
                                     static_cast<float>(command.srcRect.h)};
                const SDL_FRect fDst{static_cast<float>(command.destRect.x),
                                     static_cast<float>(command.destRect.y),
                                     static_cast<float>(command.destRect.w),
                                     static_cast<float>(command.destRect.h)};

                const SDL_FRect* src = command.hasSrcRect ? &fSrc : nullptr;
                const SDL_FRect* dst = command.hasDestRect ? &fDst : nullptr;
                SDL_RenderTexture(mRenderer, command.texture, src, dst);
                break;
            }
            case CommandType::DrawLine:
            {
                SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(
                    mRenderer, command.color.r, command.color.g, command.color.b, command.color.a);
                SDL_RenderLine(mRenderer, command.x1, command.y1, command.x2, command.y2);
                break;
            }
            case CommandType::FillRect:
            {
                SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(
                    mRenderer, command.color.r, command.color.g, command.color.b, command.color.a);
                const SDL_FRect fDst{static_cast<float>(command.destRect.x),
                                     static_cast<float>(command.destRect.y),
                                     static_cast<float>(command.destRect.w),
                                     static_cast<float>(command.destRect.h)};
                const SDL_FRect* dst = command.hasDestRect ? &fDst : nullptr;
                SDL_RenderFillRect(mRenderer, dst);
                break;
            }
            case CommandType::SetClipRect:
            {
                const SDL_Rect* clip = command.hasClipRect ? &command.clipRect : nullptr;
                SDL_SetRenderClipRect(mRenderer, clip);
                break;
            }
            case CommandType::Clear:
            {
                SDL_SetRenderDrawColor(
                    mRenderer, command.color.r, command.color.g, command.color.b, command.color.a);
                SDL_RenderClear(mRenderer);
                break;
            }
            }
        }

        mDrawCommands.clear();
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
