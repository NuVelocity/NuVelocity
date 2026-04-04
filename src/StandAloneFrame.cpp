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
            , mSource(AssetSource::Unknown)
    {
    }

    StandAloneFrame::~StandAloneFrame() = default;

    void StandAloneFrame::SetSource(AssetSource source)
    {
        mSource = source;
    }

    AssetSource StandAloneFrame::GetSource() const
    {
        return mSource;
    }

    bool StandAloneFrame::IsSourceCache() const
    {
        return mSource == AssetSource::Cache;
    }

    bool StandAloneFrame::IsSourceAsset() const
    {
        return mSource == AssetSource::SourceAsset;
    }

} // namespace nuvelocity
