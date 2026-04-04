#include "Sequence.h"

#include "BlitTypeConverter.h"
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

    Sequence::~Sequence()
    {
        for (SDL_Surface* surface : mFrameSurfaces)
        {
            if (surface != nullptr)
            {
                SDL_DestroySurface(surface);
            }
        }

        for (SDL_Texture* texture : mFrameTextures)
        {
            if (texture != nullptr)
            {
                SDL_DestroyTexture(texture);
            }
        }
    }

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
        for (SDL_Texture* texture : mFrameTextures)
        {
            if (texture != nullptr)
            {
                SDL_DestroyTexture(texture);
            }
        }
        mFrameTextures.clear();

        for (SDL_Surface* surface : mFrameSurfaces)
        {
            if (surface != nullptr)
            {
                SDL_DestroySurface(surface);
            }
        }

        mFrameSurfaces = std::move(surfaces);
        mFrameTextures.resize(mFrameSurfaces.size(), nullptr);
    }

    std::size_t Sequence::GetFrameCount() const
    {
        return mFrameSurfaces.size();
    }

    float Sequence::GetFramesPerSecond() const
    {
        return mFramesPerSecond;
    }

    SDL_Surface* Sequence::GetSurface(std::size_t index) const
    {
        if (index >= mFrameSurfaces.size())
        {
            return nullptr;
        }

        return mFrameSurfaces[index];
    }

    SDL_Texture* Sequence::GetTexture(std::size_t index, SDL_Renderer* renderer)
    {
        if (renderer == nullptr || index >= mFrameSurfaces.size())
        {
            return nullptr;
        }

        if (index >= mFrameTextures.size())
        {
            throw std::out_of_range("Sequence texture index out of range");
        }

        if (mFrameTextures[index] != nullptr)
        {
            return mFrameTextures[index];
        }

        SDL_Surface* source = mFrameSurfaces[index];
        if (source == nullptr)
        {
            return nullptr;
        }

        mFrameTextures[index] = SDL_CreateTextureFromSurface(renderer, source);
        return mFrameTextures[index];
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
