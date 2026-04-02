#ifndef NVE_FRAME_INFO_H
#define NVE_FRAME_INFO_H

#include "model/Model.h"

namespace nuvelocity
{
    class FrameInfo : public Object<FrameInfo>
    {
    public:
        FrameInfo();
        ~FrameInfo();

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFrameInfo";
            AddProperty(aInfo, "Left", &FrameInfo::mLeft);
            AddProperty(aInfo, "Top", &FrameInfo::mTop);
            AddProperty(aInfo, "Right", &FrameInfo::mRight);
            AddProperty(aInfo, "Bottom", &FrameInfo::mBottom);
            AddProperty(aInfo, "UpperLeftXOffset", &FrameInfo::mUpperLeftXOffset);
            AddProperty(aInfo, "UpperLeftYOffset", &FrameInfo::mUpperLeftYOffset);
        }

    private:
        int mLeft;
        int mTop;
        int mRight;
        int mBottom;
        int mUpperLeftXOffset;
        int mUpperLeftYOffset;
    };
} // namespace nuvelocity

#endif // NVE_FRAME_INFO_H
