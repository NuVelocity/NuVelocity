#ifndef NVE_SEQUENCE_FLAGS_H
#define NVE_SEQUENCE_FLAGS_H

#include <cstdint>

namespace nuvelocity
{
    enum class SequenceFlags : uint8_t
    {
        CenterHotSpot = 1 << 0,
        BlendedWithBlack = 1 << 1,
        CropColor0 = 1 << 2,
        Use8BitAlpha = 1 << 3,
        RunLengthEncode = 1 << 4,
        DoDither = 1 << 5,
        Lossless = 1 << 6,
        Unused = 1 << 7
    };

    inline SequenceFlags operator|(SequenceFlags lhs, SequenceFlags rhs)
    {
        return static_cast<SequenceFlags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    inline SequenceFlags operator&(SequenceFlags lhs, SequenceFlags rhs)
    {
        return static_cast<SequenceFlags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline bool HasFlag(SequenceFlags flags, SequenceFlags flag)
    {
        return (flags & flag) == flag;
    }
} // namespace nuvelocity

#endif // NVE_SEQUENCE_FLAGS_H
