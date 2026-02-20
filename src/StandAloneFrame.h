#ifndef NVE_STANDALONEFRAME_H
#define NVE_STANDALONEFRAME_H

#include <SDL3/SDL.h>
#include <string>

#include "API.h"
#include "BlitType.h"
#include "model/Model.h"

namespace nuvelocity
{
    class StandAloneFrame : public Object<StandAloneFrame>
    {
    public:
        StandAloneFrame();
        ~StandAloneFrame();

        NVE_API SDL_Texture* GetTexture(SDL_Renderer* aRenderer);

        SDL_Surface* mSurface;

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

    protected:
        SDL_Texture* mTexture;

    private:
        bool mRle;
        bool mRleAllCopy;
        bool mCropColor0;
        bool mDoDither;
        bool mLossless;
        int16_t mQuality;
        bool mCenterHotSpot;
    };
} // namespace nuvelocity

#endif // NVE_STANDALONEFRAME_H
