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

        int GetLeft() const;
        int GetTop() const;
        int GetRight() const;
        int GetBottom() const;
        int GetUpperLeftXOffset() const;
        int GetUpperLeftYOffset() const;

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CFrameInfo";
            AddProperty(info, "Left", &FrameInfo::mLeft);
            AddProperty(info, "Top", &FrameInfo::mTop);
            AddProperty(info, "Right", &FrameInfo::mRight);
            AddProperty(info, "Bottom", &FrameInfo::mBottom);
            AddProperty(info, "UpperLeftXOffset", &FrameInfo::mUpperLeftXOffset);
            AddProperty(info, "UpperLeftYOffset", &FrameInfo::mUpperLeftYOffset);
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
