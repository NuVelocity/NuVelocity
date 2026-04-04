#include "RendererSpriteBatch.h"

#include "../Image.h"

namespace nuvelocity
{
    constexpr float kCenterFactor = 0.5F;

    RendererSpriteBatch::RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window)
            : mRenderer(renderer)
            , mWindow(window)
    {
    }

    void RendererSpriteBatch::DrawImage(Image& image,
                                        const SDL_FRect* destRect,
                                        const SDL_FRect* srcRect)
    {
        if (mRenderer == nullptr)
        {
            return;
        }

        DrawCommand command{.image = &image,
                            .centered = false,
                            .hasDestRect = destRect != nullptr,
                            .destRect = destRect != nullptr ? *destRect : SDL_FRect{},
                            .hasSrcRect = srcRect != nullptr,
                            .srcRect = srcRect != nullptr ? *srcRect : SDL_FRect{}};
        mDrawCommands.push_back(command);
    }

    void RendererSpriteBatch::DrawImageCentered(Image& image)
    {
        if (mRenderer == nullptr || mWindow == nullptr)
        {
            return;
        }

        DrawCommand command{.image = &image,
                            .centered = true,
                            .hasDestRect = false,
                            .destRect = SDL_FRect{},
                            .hasSrcRect = false,
                            .srcRect = SDL_FRect{}};
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
            if (command.image == nullptr)
            {
                continue;
            }

            SDL_Texture* texture = command.image->GetTexture(mRenderer);
            if (texture == nullptr)
            {
                continue;
            }

            if (command.centered)
            {
                if (mWindow == nullptr)
                {
                    continue;
                }

                int winWidth = 0;
                int winHeight = 0;
                SDL_GetWindowSizeInPixels(mWindow, &winWidth, &winHeight);

                float texWidth = 0.0F;
                float texHeight = 0.0F;
                if (!SDL_GetTextureSize(texture, &texWidth, &texHeight))
                {
                    continue;
                }

                SDL_FRect centeredRect{
                    .x = (static_cast<float>(winWidth) - texWidth) * kCenterFactor,
                    .y = (static_cast<float>(winHeight) - texHeight) * kCenterFactor,
                    .w = texWidth,
                    .h = texHeight};

                SDL_RenderTexture(mRenderer, texture, nullptr, &centeredRect);
                continue;
            }

            const SDL_FRect* src = command.hasSrcRect ? &command.srcRect : nullptr;
            const SDL_FRect* dst = command.hasDestRect ? &command.destRect : nullptr;
            SDL_RenderTexture(mRenderer, texture, src, dst);
        }

        mDrawCommands.clear();
    }
} // namespace nuvelocity
