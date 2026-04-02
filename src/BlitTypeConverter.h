#ifndef NVE_BLIT_TYPE_CONVERTER_H
#define NVE_BLIT_TYPE_CONVERTER_H

#include "BlitType.h"
#include <optional>

namespace nuvelocity
{
    /*
     * The integer representation of the Blit Type property value has changed
     * across different Velocity Engine versions.
     *
     * (BT1) Ricochet Infinity HD, Swarm Gold, Build In Time, Costume Chaos,
     *       and Big Kahuna Reef 3 -> Text.
     * (BT1) Ricochet Infinity -> Transparent Mask is 0.
     * (BT2) Ricochet Lost Worlds -> Transparent Mask is 1.
     * (BT3) Big Kahuna Reef 1/2/Words -> Transparent Mask is 2.
     *       Removed: Normal Scale.
     * (BT4) Mosaic Tomb of Mystery -> Transparent Mask is 0.
     *       Removed: Normal Scale, Blit As Shadow.
     * (BT5) Wik and the Fable of Souls -> Transparent Mask is 1.
     *       Removed: Normal Scale, Blit As Shadow.
     * (BT6 TODO) Ricochet Xtreme, Swarm -> Values in use: 2, 5, and 6.
     */
    enum class BlitTypeRevision
    {
        Type1,
        Type2,
        Type3,
        Type4,
        Type5
    };

    class BlitTypeConverter
    {
    public:
        static std::optional<::BlitType> Int32ToType1(int type)
        {
            switch (type)
            {
            case 0:
                return BLIT_TRANSPARENT_MASK;
            case 1:
                return BLIT_NORMAL;
            case 2:
                return BLIT_BLEND_BLACK_BIAS;
            case 3:
                return BLIT_BLEND_TEST_LIGHT;
            case 4:
                return BLIT_BLIT_AS_SHADOW;
            case 5:
                return BLIT_NORMAL_SCALE;
            case 6:
                return BLIT_AMPLIFY_LIGHT;
            default:
                return std::nullopt;
            }
        }

        static std::optional<::BlitType> Int32ToType2(int type)
        {
            switch (type)
            {
            case 1:
                return BLIT_TRANSPARENT_MASK;
            case 0:
                return BLIT_NORMAL;
            case 2:
                return BLIT_BLEND_BLACK_BIAS;
            case 3:
                return BLIT_BLEND_TEST_LIGHT;
            case 4:
                return BLIT_BLIT_AS_SHADOW;
            case 5:
                return BLIT_NORMAL_SCALE;
            case 6:
                return BLIT_AMPLIFY_LIGHT;
            default:
                return std::nullopt;
            }
        }

        static std::optional<::BlitType> Int32ToType3(int type)
        {
            switch (type)
            {
            case 2:
                return BLIT_TRANSPARENT_MASK;
            case 3:
                return BLIT_NORMAL;
            case 0:
                return BLIT_BLEND_BLACK_BIAS;
            case 1:
                return BLIT_BLEND_TEST_LIGHT;
            case 4:
                return BLIT_BLIT_AS_SHADOW;
            // BLIT_NORMAL_SCALE was removed for this revision.
            case 5:
                return BLIT_AMPLIFY_LIGHT;
            default:
                return std::nullopt;
            }
        }

        static std::optional<::BlitType> Int32ToType4(int type)
        {
            switch (type)
            {
            case 0:
                return BLIT_TRANSPARENT_MASK;
            case 1:
                return BLIT_NORMAL;
            case 3:
                return BLIT_BLEND_BLACK_BIAS;
            case 4:
                return BLIT_BLEND_TEST_LIGHT;
            // BLIT_BLIT_AS_SHADOW and BLIT_NORMAL_SCALE were removed for this revision.
            case 2:
                return BLIT_AMPLIFY_LIGHT;
            default:
                return std::nullopt;
            }
        }

        static std::optional<::BlitType> Int32ToType5(int type)
        {
            switch (type)
            {
            case 1:
                return BLIT_TRANSPARENT_MASK;
            case 0:
                return BLIT_NORMAL;
            case 2:
                return BLIT_BLEND_BLACK_BIAS;
            case 3:
                return BLIT_BLEND_TEST_LIGHT;
            // BLIT_BLIT_AS_SHADOW and BLIT_NORMAL_SCALE were removed for this revision.
            case 4:
                return BLIT_AMPLIFY_LIGHT;
            default:
                return std::nullopt;
            }
        }

        static std::optional<::BlitType> Int32ToType(int type, BlitTypeRevision revision)
        {
            switch (revision)
            {
            case BlitTypeRevision::Type1:
                return Int32ToType1(type);
            case BlitTypeRevision::Type2:
                return Int32ToType2(type);
            case BlitTypeRevision::Type3:
                return Int32ToType3(type);
            case BlitTypeRevision::Type4:
                return Int32ToType4(type);
            case BlitTypeRevision::Type5:
                return Int32ToType5(type);
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToType1(::BlitType type)
        {
            switch (type)
            {
            case BLIT_TRANSPARENT_MASK:
                return 0;
            case BLIT_NORMAL:
                return 1;
            case BLIT_BLEND_BLACK_BIAS:
                return 2;
            case BLIT_BLEND_TEST_LIGHT:
                return 3;
            case BLIT_BLIT_AS_SHADOW:
                return 4;
            case BLIT_NORMAL_SCALE:
                return 5;
            case BLIT_AMPLIFY_LIGHT:
                return 6;
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToType2(::BlitType type)
        {
            switch (type)
            {
            case BLIT_TRANSPARENT_MASK:
                return 1;
            case BLIT_NORMAL:
                return 0;
            case BLIT_BLEND_BLACK_BIAS:
                return 2;
            case BLIT_BLEND_TEST_LIGHT:
                return 3;
            case BLIT_BLIT_AS_SHADOW:
                return 4;
            case BLIT_NORMAL_SCALE:
                return 5;
            case BLIT_AMPLIFY_LIGHT:
                return 6;
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToType3(::BlitType type)
        {
            switch (type)
            {
            case BLIT_TRANSPARENT_MASK:
                return 2;
            case BLIT_NORMAL:
                return 3;
            case BLIT_BLEND_BLACK_BIAS:
                return 0;
            case BLIT_BLEND_TEST_LIGHT:
                return 1;
            case BLIT_BLIT_AS_SHADOW:
                return 4;
            // BLIT_NORMAL_SCALE has no mapping for this revision.
            case BLIT_AMPLIFY_LIGHT:
                return 5;
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToType4(::BlitType type)
        {
            switch (type)
            {
            case BLIT_TRANSPARENT_MASK:
                return 0;
            case BLIT_NORMAL:
                return 1;
            case BLIT_BLEND_BLACK_BIAS:
                return 3;
            case BLIT_BLEND_TEST_LIGHT:
                return 4;
            // BLIT_BLIT_AS_SHADOW and BLIT_NORMAL_SCALE have no mappings for this revision.
            case BLIT_AMPLIFY_LIGHT:
                return 2;
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToType5(::BlitType type)
        {
            switch (type)
            {
            case BLIT_TRANSPARENT_MASK:
                return 1;
            case BLIT_NORMAL:
                return 0;
            case BLIT_BLEND_BLACK_BIAS:
                return 2;
            case BLIT_BLEND_TEST_LIGHT:
                return 3;
            // BLIT_BLIT_AS_SHADOW and BLIT_NORMAL_SCALE have no mappings for this revision.
            case BLIT_AMPLIFY_LIGHT:
                return 4;
            default:
                return std::nullopt;
            }
        }

        static std::optional<int> ToInt(::BlitType type, BlitTypeRevision revision)
        {
            switch (revision)
            {
            case BlitTypeRevision::Type1:
                return ToType1(type);
            case BlitTypeRevision::Type2:
                return ToType2(type);
            case BlitTypeRevision::Type3:
                return ToType3(type);
            case BlitTypeRevision::Type4:
                return ToType4(type);
            case BlitTypeRevision::Type5:
                return ToType5(type);
            default:
                return std::nullopt;
            }
        }
    };
} // namespace nuvelocity

#endif // NVE_BLIT_TYPE_CONVERTER_H
