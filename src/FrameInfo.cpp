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
} // namespace nuvelocity
