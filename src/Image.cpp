#include "Image.h"

#include "Sequence.h"
#include "StandAloneFrame.h"

#include <cmath>

namespace nuvelocity
{
    Image::Image()
            : mKind(Kind::None)
            , mSequence(nullptr)
            , mFrame(nullptr)
            , mAnimationStartTick(0)
    {
    }

    Image::Image(Sequence& sequence)
            : mKind(Kind::Sequence)
            , mSequence(&sequence)
            , mFrame(nullptr)
            , mAnimationStartTick(SDL_GetTicks())
    {
    }

    Image::Image(StandAloneFrame& frame)
            : mKind(Kind::StandAloneFrame)
            , mSequence(nullptr)
            , mFrame(&frame)
            , mAnimationStartTick(0)
    {
    }

    bool Image::IsValid() const
    {
        return mKind != Kind::None;
    }

    bool Image::IsSequence() const
    {
        return mKind == Kind::Sequence;
    }

    bool Image::IsStandAloneFrame() const
    {
        return mKind == Kind::StandAloneFrame;
    }

    void Image::ResetAnimation()
    {
        if (mKind == Kind::Sequence)
        {
            mAnimationStartTick = SDL_GetTicks();
        }
    }

    std::size_t Image::GetFrameCount() const
    {
        if (mKind == Kind::Sequence && mSequence != nullptr)
        {
            return mSequence->GetFrameCount();
        }

        if (mKind == Kind::StandAloneFrame && mFrame != nullptr)
        {
            return 1;
        }

        return 0;
    }

    std::size_t Image::GetAnimatedFrameIndex() const
    {
        if (mKind != Kind::Sequence || mSequence == nullptr)
        {
            return 0;
        }

        const std::size_t frameCount = mSequence->GetFrameCount();
        if (frameCount == 0)
        {
            return 0;
        }

        const double fps = static_cast<double>(mSequence->GetFramesPerSecond());
        if (fps <= 0.0)
        {
            return 0;
        }

        const uint64_t now = SDL_GetTicks();
        const uint64_t elapsed = now - mAnimationStartTick;
        const double frameProgress = (static_cast<double>(elapsed) * fps) / 1000.0;
        const std::size_t frameIndex = static_cast<std::size_t>(std::floor(frameProgress));
        return frameIndex % frameCount;
    }

    SDL_Surface* Image::GetSurface() const
    {
        if (mKind == Kind::Sequence && mSequence != nullptr)
        {
            return mSequence->GetSurface(GetAnimatedFrameIndex());
        }

        if (mKind == Kind::StandAloneFrame && mFrame != nullptr)
        {
            return mFrame->GetSurface();
        }

        return nullptr;
    }

    SDL_Texture* Image::GetTexture(SDL_Renderer* renderer)
    {
        if (mKind == Kind::Sequence && mSequence != nullptr)
        {
            return mSequence->GetTexture(GetAnimatedFrameIndex(), renderer);
        }

        if (mKind == Kind::StandAloneFrame && mFrame != nullptr)
        {
            return mFrame->GetTexture(renderer);
        }

        return nullptr;
    }
} // namespace nuvelocity
