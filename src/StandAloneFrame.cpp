#include "StandAloneFrame.h"

namespace nuvelocity
{
    constexpr int kDefaultQuality = 80;

    StandAloneFrame::StandAloneFrame()
            : mRle(true)
            , mRleAllCopy(false)
            , mCropColor0(true)
            , mDoDither(true)
            , mLossless(false)
            , mQuality(kDefaultQuality)
            , mCenterHotSpot(false)
            , mSurface(nullptr)
            , mTexture(nullptr)
    {
    }

    StandAloneFrame::~StandAloneFrame()
    {
        if (mSurface != nullptr)
        {
            SDL_DestroySurface(mSurface);
        }
        if (mTexture != nullptr)
        {
            SDL_DestroyTexture(mTexture);
        }
    }

    SDL_Texture* StandAloneFrame::GetTexture(SDL_Renderer* aRenderer)
    {
        if (mTexture != nullptr)
        {
            return mTexture;
        }
        if (mSurface != nullptr)
        {
            mTexture = SDL_CreateTextureFromSurface(aRenderer, mSurface);
            return mTexture;
        }
        return nullptr;
    }

    bool StandAloneFrame::Render(SDL_Renderer* renderer, const SDL_FRect* destRect,
                                 const SDL_FRect* srcRect)
    {
        SDL_Texture* texture = GetTexture(renderer);
        if (texture == nullptr)
        {
            return false;
        }

        return SDL_RenderTexture(renderer, texture, srcRect, destRect);
    }

    bool StandAloneFrame::RenderCentered(SDL_Renderer* renderer, SDL_Window* window)
    {
        if (renderer == nullptr || window == nullptr)
        {
            return false;
        }

        SDL_Texture* texture = GetTexture(renderer);
        if (texture == nullptr)
        {
            return false;
        }

        int winWidth = 0;
        int winHeight = 0;
        SDL_GetWindowSizeInPixels(window, &winWidth, &winHeight);

        float texWidth = 0.0F;
        float texHeight = 0.0F;
        if (!SDL_GetTextureSize(texture, &texWidth, &texHeight))
        {
            return false;
        }

        SDL_FRect destRect = {(static_cast<float>(winWidth) - texWidth) / 2.0F,
                              (static_cast<float>(winHeight) - texHeight) / 2.0F, texWidth,
                              texHeight};
        return SDL_RenderTexture(renderer, texture, nullptr, &destRect);
    }
} // namespace nuvelocity
