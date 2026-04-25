#ifndef NVE_BLITTYPE_H
#define NVE_BLITTYPE_H

#include <array>
#include <cstdint>
#include <utility>

namespace nuvelocity
{
    inline constexpr int kBlitTypeCount = 7;

    enum BlitType : uint8_t
    {
        BlitTransparentMask = 0,
        BlitNormal = 1,
        BlitBlendBlackBias = 2,
        BlitBlendTestLight = 3,
        BlitBlitAsShadow = 4,
        BlitNormalScale = 5,
        BlitAmplifyLight = 6
    };

    inline constexpr std::array<std::pair<int, const char*>, 7> kBlitTypeSerializedValues = {{
        {BlitTransparentMask, "Transparent Mask"},
        {BlitNormal, "Normal"},
        {BlitBlendBlackBias, "Blend Black Bias"},
        {BlitBlendTestLight, "Blend Test Light"},
        {BlitBlitAsShadow, "Blit As Shadow"},
        {BlitNormalScale, "Normal Scale"},
        {BlitAmplifyLight, "Blend Amplify Light"},
    }};

    inline const std::array<std::pair<int, const char*>, kBlitTypeCount>&
    GetBlitTypeSerializedValues()
    {
        return kBlitTypeSerializedValues;
    }
} // namespace nuvelocity

#endif // NVE_BLITTYPE_H
