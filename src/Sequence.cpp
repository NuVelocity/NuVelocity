#include "Sequence.h"

#include "BlitTypeConverter.h"

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

    void Sequence::ApplyFrameInfoList(SequenceFlags flags, int rawBlitType, float framesPerSecond,
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
