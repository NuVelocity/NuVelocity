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
} // namespace nuvelocity
