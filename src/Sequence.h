#ifndef NVE_SEQUENCE_H
#define NVE_SEQUENCE_H

#include "API.h"
#include "AssetSource.h"
#include "BlitType.h"
#include "Frame.h"
#include "Object.h"
#include "SequenceFlags.h"
#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nuvelocity
{
    enum class BlitTypeRevision : uint8_t;

    enum class ImagePropertyListFormat : uint8_t
    {
        Format1,
        Format2,
        Format3
    };

    inline constexpr float kSequenceDefaultFramesPerSecond = 15.0F;

    class Sequence : public Object<Sequence>
    {
    public:
        NVE_API Sequence();
        NVE_API explicit Sequence(ImagePropertyListFormat format);
        NVE_API ~Sequence();

        NVE_API int GetXOffset() const;
        NVE_API int GetYOffset() const;
        NVE_API bool GetCenterHotSpot() const;

        NVE_API void SetFrames(std::vector<std::unique_ptr<Frame>>&& frames);

        NVE_API SDL_Point GetAnchor() const;
        NVE_API void SetAnchor(int x, int y);

        NVE_API std::size_t GetFrameCount() const;
        NVE_API float GetFramesPerSecond() const;
        NVE_API Frame* GetFrame(std::size_t index) const;
        NVE_API SDL_Surface* GetSurface(std::size_t index) const;
        NVE_API SDL_Texture* GetTexture(std::size_t index, SDL_Renderer* renderer) const;

        NVE_API void SetSource(AssetSource source);
        NVE_API AssetSource GetSource() const;
        NVE_API bool IsSourceCache() const;
        NVE_API bool IsSourceAsset() const;
        NVE_API bool GetDoDither() const
        {
            return mDoDither;
        }

#ifdef NVE_RESTORE_TGA
        NVE_API void SetRawListText(std::string text);
        NVE_API const std::string& GetRawListText() const;
        NVE_API void SetSpriteAtlas(SDL_Surface* atlas);
        NVE_API SDL_Surface* GetSpriteAtlas() const;
#endif

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CSequence";

            AddProperty(info, "Frames Per Second", &Sequence::mFramesPerSecond);
            AddEnumProperty<BlitType>(
                info, "Blit Type", &Sequence::mBlitType, GetBlitTypeSerializedValues());
            AddProperty(info, "X Offset", &Sequence::mXOffset);
            AddProperty(info, "Y Offset", &Sequence::mYOffset);
            AddProperty(info, "Center Hot Spot", &Sequence::mCenterHotSpot);
            AddProperty(info, "Blended With Black", &Sequence::mBlendedWithBlack);
            AddProperty(info, "Crop Clor 0", &Sequence::mCropAlphaChannel);
            AddProperty(info, "Use 8 Bit Alpha", &Sequence::mUse8BitAlpha);
            AddProperty(info, "Do Dither", &Sequence::mDoDither);
            AddProperty(info, "Loss Less", &Sequence::mIsLossless);
            AddProperty(info, "Quality", &Sequence::mJpegQuality);

#if 0
            // TN: Exclusive to Lionheart.
            AddProperty(info, "Menu Position", &Sequence::mMenuPosition);

            // TN: Exclusive to Ricochet Lost Worlds and Ricochet Infinity.
            AddProperty(info, "Sequence of Coordinates", &Sequence::mSequenceOfCoordinates);

            // TN: Exclusive to Build In Time and Costume Chaos.
            AddProperty(info, "Y-Sort", &Sequence::mYSort);

            // TN: Exclusive to Build In Time.
            AddProperty(info, "Poke Audio", &Sequence::mPokeAudio);

            // TN: Exclusive to Costume Chaos.
            AddProperty(info, "Editor Only", &Sequence::mEditorOnly);

            AddProperty(info, "Use Every", &Sequence::mUseEvery);
            AddProperty(info, "Always Include Last Frame", &Sequence::mAlwaysIncludeLastFrame);
            AddProperty(info, "Crop Color 0", &Sequence::mCropAlphaChannel);
            AddProperty(info, "Run Length Encode", &Sequence::mIsRle);

            // TN: Present in Star Trek Away Team sequence files.
            AddProperty(info, "Dither", &Sequence::mDoDither);

            AddProperty(info, "Loss Less 2", &Sequence::mIsLossless);
            AddProperty(info, "Quality2", &Sequence::mJpegQuality);
            AddProperty(info, "JPEG Quality", &Sequence::mJpegQuality);
            AddProperty(info, "JPEG Quality 2", &Sequence::mJpegQuality);
            AddProperty(info, "DDS", &Sequence::mIsDds);
            AddProperty(info, "Needs Buffer", &Sequence::mNeedsBuffer);
            // TN: Present in Swarm Gold, Ricochet Infinity HD, Big Kahuna Reef 3,
            // Build In Time, and Costume Chaos.
            AddProperty(info, "Mipmap For Native Version", &Sequence::mMipmapForNativeVersion);
#endif
        }

        NVE_API void ApplySequenceFlags(SequenceFlags flags);
        NVE_API void ApplyFrameInfoList(SequenceFlags flags,
                                        int rawBlitType,
                                        float framesPerSecond,
                                        BlitTypeRevision revision);

    private:
#if 0
        bool mHasFixedCropColor0Name;
        bool mHasDdsSupport;
        bool mHasMipmapSupport;

        ImagePropertyListFormat mFormat;

        std::string mMenuPosition;
        std::string mSequenceOfCoordinates;
        int mYSort;
        std::string mPokeAudio;
        bool mEditorOnly;
#endif

        float mFramesPerSecond;
        BlitType mBlitType;
        int mXOffset;
        int mYOffset;
#if 0
        int mUseEvery;
        bool mAlwaysIncludeLastFrame;
#endif
        bool mCenterHotSpot;
        bool mBlendedWithBlack;
        bool mCropAlphaChannel;
        bool mUse8BitAlpha;
#if 0
        bool mIsRle;
#endif
        bool mDoDither;
        bool mIsLossless;
        int mJpegQuality;

        std::vector<std::unique_ptr<Frame>> mFrames;
        SDL_Point mAnchor;
        AssetSource mSource;

#ifdef NVE_RESTORE_TGA
        std::string mRawListText;
        SDL_Surface* mSpriteAtlas = nullptr;
#endif

#if 0
        bool mIsDds;
        bool mNeedsBuffer;
        bool mMipmapForNativeVersion;
#endif
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCE_H
