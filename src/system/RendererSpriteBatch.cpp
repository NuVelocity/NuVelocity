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

    void RendererSpriteBatch::Draw(SDL_Surface* surface,
                                   const SDL_FRect* destRect,
                                   const SDL_FRect* srcRect,
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
                            .destRect = destRect != nullptr ? *destRect : SDL_FRect{},
                            .hasSrcRect = srcRect != nullptr,
                            .srcRect = srcRect != nullptr ? *srcRect : SDL_FRect{},
                            .color = color};
        mDrawCommands.push_back(command);
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
                            .destRect = SDL_FRect{},
                            .hasSrcRect = false,
                            .srcRect = SDL_FRect{},
                            .color = {255, 255, 255, 255}};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::DrawLine(
        float x1, float y1, float x2, float y2, SDL_Color color, float thickness)
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
                            .thickness = thickness};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::FillRect(const SDL_FRect* rect, SDL_Color color)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.type = CommandType::FillRect,
                            .hasDestRect = rect != nullptr,
                            .destRect = rect != nullptr ? *rect : SDL_FRect{},
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

                SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, command.surface);
                if (texture == nullptr)
                    continue;

                SDL_SetTextureColorMod(texture, command.color.r, command.color.g, command.color.b);
                SDL_SetTextureAlphaMod(texture, command.color.a);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

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

                    SDL_FRect centeredRect{
                        .x = (static_cast<float>(winWidth) - texWidth) * kCenterFactor,
                        .y = (static_cast<float>(winHeight) - texHeight) * kCenterFactor,
                        .w = texWidth,
                        .h = texHeight};

                    SDL_RenderTexture(mRenderer, texture, nullptr, &centeredRect);
                }
                else
                {
                    const SDL_FRect* src = command.hasSrcRect ? &command.srcRect : nullptr;
                    const SDL_FRect* dst = command.hasDestRect ? &command.destRect : nullptr;
                    SDL_RenderTexture(mRenderer, texture, src, dst);
                }
                SDL_DestroyTexture(texture);
                break;
            }
            case CommandType::DrawTexture:
            {
                if (command.texture == nullptr)
                    continue;

                SDL_SetTextureColorMod(
                    command.texture, command.color.r, command.color.g, command.color.b);
                SDL_SetTextureAlphaMod(command.texture, command.color.a);
                SDL_SetTextureBlendMode(command.texture, SDL_BLENDMODE_BLEND);

                const SDL_FRect* src = command.hasSrcRect ? &command.srcRect : nullptr;
                const SDL_FRect* dst = command.hasDestRect ? &command.destRect : nullptr;
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
                const SDL_FRect* dst = command.hasDestRect ? &command.destRect : nullptr;
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
