#include "Sequence.h"

#include "BlitTypeConverter.h"
#include "Frame.h"
#include <stdexcept>
#include <utility>

namespace nuvelocity
{
#if 0
    constexpr int kDefaultUseEvery = 1;
#endif
    constexpr int kDefaultJpegQuality = 80;

    Sequence::Sequence(ImagePropertyListFormat format)
#if 0
            : mHasFixedCropColor0Name(false)
            , mHasDdsSupport(false)
            , mHasMipmapSupport(false)
            , mFormat(format)
            , mMenuPosition("")
            , mSequenceOfCoordinates("")
            , mYSort(0)
            , mPokeAudio("")
            , mEditorOnly(false)
            , mFramesPerSecond(kSequenceDefaultFramesPerSecond)
            , mBlitType(BLIT_TRANSPARENT_MASK)
            , mXOffset(0)
            , mYOffset(0)
            , mUseEvery(kDefaultUseEvery)
            , mAlwaysIncludeLastFrame(false)
            , mCenterHotSpot(true)
            , mBlendedWithBlack(true)
            , mCropAlphaChannel(true)
            , mUse8BitAlpha(false)
            , mIsRle(true)
            , mDoDither(true)
            , mIsLossless(false)
            , mJpegQuality(kDefaultJpegQuality)
            , mIsDds(false)
            , mNeedsBuffer(false)
            , mMipmapForNativeVersion(true)
#else
            : mFramesPerSecond(kSequenceDefaultFramesPerSecond)
            , mBlitType(BLIT_TRANSPARENT_MASK)
            , mXOffset(0)
            , mYOffset(0)
            , mCenterHotSpot(true)
            , mBlendedWithBlack(true)
            , mCropAlphaChannel(true)
            , mUse8BitAlpha(false)
            , mDoDither(true)
            , mIsLossless(false)
            , mJpegQuality(kDefaultJpegQuality)
#endif
    {
#if 0
        // mFormat(format) uses this parameter in the legacy layout.
#else
        (void)format;
#endif
    }

    Sequence::Sequence()
            : Sequence(ImagePropertyListFormat::Format3)
    {
    }

    Sequence::~Sequence() = default;

    int Sequence::GetXOffset() const
    {
        return mXOffset;
    }

    int Sequence::GetYOffset() const
    {
        return mYOffset;
    }

    bool Sequence::GetCenterHotSpot() const
    {
        return mCenterHotSpot;
    }

    void Sequence::SetFrames(std::vector<SDL_Surface*>&& surfaces)
    {
        mFrames.clear();
        mFrames.reserve(surfaces.size());

        for (SDL_Surface* surface : surfaces)
        {
            auto frame = std::make_unique<Frame>();
            frame->SetSurface(surface);
            mFrames.push_back(std::move(frame));
        }
    }

    std::size_t Sequence::GetFrameCount() const
    {
        return mFrames.size();
    }

    float Sequence::GetFramesPerSecond() const
    {
        return mFramesPerSecond;
    }

    Frame* Sequence::GetFrame(std::size_t index) const
    {
        if (index >= mFrames.size())
        {
            return nullptr;
        }

        return mFrames[index].get();
    }

    SDL_Surface* Sequence::GetSurface(std::size_t index) const
    {
        Frame* frame = GetFrame(index);
        if (frame == nullptr)
        {
            return nullptr;
        }

        return frame->GetSurface();
    }

    SDL_Texture* Sequence::GetTexture(std::size_t index, SDL_Renderer* renderer) const
    {
        Frame* frame = GetFrame(index);
        if (frame == nullptr)
        {
            return nullptr;
        }

        return frame->GetTexture(renderer);
    }

    void Sequence::ApplySequenceFlags(SequenceFlags flags)
    {
        mCenterHotSpot = HasFlag(flags, SequenceFlags::CenterHotSpot);
        mBlendedWithBlack = HasFlag(flags, SequenceFlags::BlendedWithBlack);
        mCropAlphaChannel = HasFlag(flags, SequenceFlags::CropColor0);
        mUse8BitAlpha = HasFlag(flags, SequenceFlags::Use8BitAlpha);
#if 0
        mIsRle = HasFlag(flags, SequenceFlags::RunLengthEncode);
#endif
        mDoDither = HasFlag(flags, SequenceFlags::DoDither);
        mIsLossless = HasFlag(flags, SequenceFlags::Lossless);
    }

    void Sequence::ApplyFrameInfoList(SequenceFlags flags,
                                      int rawBlitType,
                                      float framesPerSecond,
                                      BlitTypeRevision revision)
    {
        ApplySequenceFlags(flags);

        const std::optional<BlitType> converted =
            BlitTypeConverter::Int32ToType(rawBlitType, revision);
        if (converted.has_value())
        {
            mBlitType = converted.value();
        }

        mFramesPerSecond = framesPerSecond;
    }
} // namespace nuvelocity
