#ifndef NVE_BLITTYPE_H
#define NVE_BLITTYPE_H

#include <array>
#include <utility>

namespace nuvelocity
{
    enum BlitType
    {
        BLIT_TRANSPARENT_MASK = 0,
        BLIT_NORMAL = 1,
        BLIT_BLEND_BLACK_BIAS = 2,
        BLIT_BLEND_TEST_LIGHT = 3,
        BLIT_BLIT_AS_SHADOW = 4,
        BLIT_NORMAL_SCALE = 5,
        BLIT_AMPLIFY_LIGHT = 6
    };

    inline constexpr std::array<std::pair<int, const char*>, 7> kBlitTypeSerializedValues = {{
        {BLIT_TRANSPARENT_MASK, "Transparent Mask"},
        {BLIT_NORMAL, "Normal"},
        {BLIT_BLEND_BLACK_BIAS, "Blend Black Bias"},
        {BLIT_BLEND_TEST_LIGHT, "Blend Test Light"},
        {BLIT_BLIT_AS_SHADOW, "Blit As Shadow"},
        {BLIT_NORMAL_SCALE, "Normal Scale"},
        {BLIT_AMPLIFY_LIGHT, "Blend Amplify Light"},
    }};

    inline const std::array<std::pair<int, const char*>, 7>& GetBlitTypeSerializedValues()
    {
        return kBlitTypeSerializedValues;
    }
} // namespace nuvelocity

#endif // NVE_BLITTYPE_H
