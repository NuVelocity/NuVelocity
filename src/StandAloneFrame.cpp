#include "StandAloneFrame.h"

namespace nuvelocity
{
    constexpr int kDefaultQuality = 80;

    StandAloneFrame::StandAloneFrame()
            : mRle(true)
            , mRleAllCopy(false)
            , mCropColor0(true)
            , mDoDither(true)
            , mLossless(false)
            , mQuality(kDefaultQuality)
            , mCenterHotSpot(false)
    {
    }

    StandAloneFrame::~StandAloneFrame() = default;

} // namespace nuvelocity
