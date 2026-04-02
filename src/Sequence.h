#ifndef NVE_SEQUENCE_H
#define NVE_SEQUENCE_H

#include "BlitType.h"
#include "SequenceFlags.h"
#include "model/Model.h"
#include <SDL3/SDL.h>
#include <cstddef>
#include <string>
#include <vector>

namespace nuvelocity
{
    enum class BlitTypeRevision;

    enum class ImagePropertyListFormat
    {
        Format1,
        Format2,
        Format3
    };

    inline constexpr float kSequenceDefaultFramesPerSecond = 15.0F;

    class Sequence : public Object<Sequence>
    {
    public:
        Sequence();
        explicit Sequence(ImagePropertyListFormat format);
        ~Sequence();

        int GetXOffset() const;
        int GetYOffset() const;
        bool GetCenterHotSpot() const;

        void SetFrames(std::vector<SDL_Surface*>&& surfaces);
        std::size_t GetFrameCount() const;
        SDL_Surface* GetSurface(std::size_t index) const;
        SDL_Texture* GetTexture(std::size_t index, SDL_Renderer* renderer);

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CSequence";

            AddProperty(aInfo, "Frames Per Second", &Sequence::mFramesPerSecond);
            AddEnumProperty<BlitType>(aInfo, "Blit Type", &Sequence::mBlitType,
                            GetBlitTypeSerializedValues());
            AddProperty(aInfo, "X Offset", &Sequence::mXOffset);
            AddProperty(aInfo, "Y Offset", &Sequence::mYOffset);
            AddProperty(aInfo, "Center Hot Spot", &Sequence::mCenterHotSpot);
            AddProperty(aInfo, "Blended With Black", &Sequence::mBlendedWithBlack);
            AddProperty(aInfo, "Crop Clor 0", &Sequence::mCropAlphaChannel);
            AddProperty(aInfo, "Use 8 Bit Alpha", &Sequence::mUse8BitAlpha);
            AddProperty(aInfo, "Do Dither", &Sequence::mDoDither);
            AddProperty(aInfo, "Loss Less", &Sequence::mIsLossless);
            AddProperty(aInfo, "Quality", &Sequence::mJpegQuality);

#if 0
            // TN: Exclusive to Lionheart.
            AddProperty(aInfo, "Menu Position", &Sequence::mMenuPosition);

            // TN: Exclusive to Ricochet Lost Worlds and Ricochet Infinity.
            AddProperty(aInfo, "Sequence of Coordinates", &Sequence::mSequenceOfCoordinates);

            // TN: Exclusive to Build In Time and Costume Chaos.
            AddProperty(aInfo, "Y-Sort", &Sequence::mYSort);

            // TN: Exclusive to Build In Time.
            AddProperty(aInfo, "Poke Audio", &Sequence::mPokeAudio);

            // TN: Exclusive to Costume Chaos.
            AddProperty(aInfo, "Editor Only", &Sequence::mEditorOnly);

            AddProperty(aInfo, "Use Every", &Sequence::mUseEvery);
            AddProperty(aInfo, "Always Include Last Frame", &Sequence::mAlwaysIncludeLastFrame);
            AddProperty(aInfo, "Crop Color 0", &Sequence::mCropAlphaChannel);
            AddProperty(aInfo, "Run Length Encode", &Sequence::mIsRle);

            // TN: Present in Star Trek Away Team sequence files.
            AddProperty(aInfo, "Dither", &Sequence::mDoDither);

            AddProperty(aInfo, "Loss Less 2", &Sequence::mIsLossless);
            AddProperty(aInfo, "Quality2", &Sequence::mJpegQuality);
            AddProperty(aInfo, "JPEG Quality", &Sequence::mJpegQuality);
            AddProperty(aInfo, "JPEG Quality 2", &Sequence::mJpegQuality);
            AddProperty(aInfo, "DDS", &Sequence::mIsDds);
            AddProperty(aInfo, "Needs Buffer", &Sequence::mNeedsBuffer);
            // TN: Present in Swarm Gold, Ricochet Infinity HD, Big Kahuna Reef 3,
            // Build In Time, and Costume Chaos.
            AddProperty(aInfo, "Mipmap For Native Version", &Sequence::mMipmapForNativeVersion);
#endif
        }

        void ApplySequenceFlags(SequenceFlags flags);
        void ApplyFrameInfoList(SequenceFlags flags, int rawBlitType, float framesPerSecond,
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

        std::vector<SDL_Surface*> mFrameSurfaces;
        std::vector<SDL_Texture*> mFrameTextures;

#if 0
        bool mIsDds;
        bool mNeedsBuffer;
        bool mMipmapForNativeVersion;
#endif
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCE_H
