#include "FrameInfo.h"

namespace nuvelocity
{
    FrameInfo::FrameInfo()
            : mLeft(0)
            , mTop(0)
            , mRight(0)
            , mBottom(0)
            , mUpperLeftXOffset(0)
            , mUpperLeftYOffset(0)
    {
    }

    FrameInfo::~FrameInfo() = default;

    int FrameInfo::GetLeft() const
    {
        return mLeft;
    }

    int FrameInfo::GetTop() const
    {
        return mTop;
    }

    int FrameInfo::GetRight() const
    {
        return mRight;
    }

    int FrameInfo::GetBottom() const
    {
        return mBottom;
    }

    int FrameInfo::GetUpperLeftXOffset() const
    {
        return mUpperLeftXOffset;
    }

    int FrameInfo::GetUpperLeftYOffset() const
    {
        return mUpperLeftYOffset;
    }
} // namespace nuvelocity
