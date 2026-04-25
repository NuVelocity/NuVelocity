#ifndef NVE_STANDALONEFRAME_H
#define NVE_STANDALONEFRAME_H

#include <SDL3/SDL.h>
#include <string>

#include "API.h"
#include "AssetSource.h"
#include "BlitType.h"
#include "Frame.h"
#include "model/Model.h"

namespace nuvelocity
{
    class StandAloneFrame : public Object<StandAloneFrame, Frame>
    {
    public:
        StandAloneFrame();
        ~StandAloneFrame();

        void SetSource(AssetSource source);
        AssetSource GetSource() const;
        bool IsSourceCache() const;
        bool IsSourceAsset() const;

        bool GetRle() const
        {
            return mRle;
        }
        bool GetRleAllCopy() const
        {
            return mRleAllCopy;
        }
        bool GetCropColor0() const
        {
            return mCropColor0;
        }
        bool GetDoDither() const
        {
            return mDoDither;
        }
        bool GetLossless() const
        {
            return mLossless;
        }
        int16_t GetQuality() const
        {
            return mQuality;
        }
        bool GetCenterHotSpot() const
        {
            return mCenterHotSpot;
        }
        void SetRle(bool value)
        {
            mRle = value;
        }
        void SetRleAllCopy(bool value)
        {
            mRleAllCopy = value;
        }
        void SetCropColor0(bool value)
        {
            mCropColor0 = value;
        }
        void SetDoDither(bool value)
        {
            mDoDither = value;
        }
        void SetLossless(bool value)
        {
            mLossless = value;
        }
        void SetQuality(int16_t value)
        {
            mQuality = value;
        }
        void SetCenterHotSpot(bool value)
        {
            mCenterHotSpot = value;
        }

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CStandAloneFrame";
            AddProperty(aInfo, "Run Length Encode", &StandAloneFrame::mRle);
            AddProperty(aInfo, "RLE All Copy", &StandAloneFrame::mRleAllCopy);
            AddProperty(aInfo, "Crop Color 0", &StandAloneFrame::mCropColor0);
            AddProperty(aInfo, "Do Dither", &StandAloneFrame::mDoDither);
            AddProperty(aInfo, "Loss Less", &StandAloneFrame::mLossless);
            AddProperty(aInfo, "Quality", &StandAloneFrame::mQuality);
            AddProperty(aInfo, "Center Hot Spot", &StandAloneFrame::mCenterHotSpot);
        }

    private:
        bool mRle;
        bool mRleAllCopy;
        bool mCropColor0;
        bool mDoDither;
        bool mLossless;
        int16_t mQuality;
        bool mCenterHotSpot;
        AssetSource mSource;
    };
} // namespace nuvelocity

#endif // NVE_STANDALONEFRAME_H
