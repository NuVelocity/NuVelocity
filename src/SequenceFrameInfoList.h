#ifndef NVE_SEQUENCE_FRAME_INFO_LIST_H
#define NVE_SEQUENCE_FRAME_INFO_LIST_H

#include "BlitTypeConverter.h"
#include "FrameInfo.h"
#include "Object.h"
#include "Sequence.h"
#include "SequenceFlags.h"
#include <vector>

namespace nuvelocity
{
    class SequenceFrameInfoList : public Object<SequenceFrameInfoList>
    {
    public:
        SequenceFrameInfoList();
        ~SequenceFrameInfoList();

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CSequenceFrameInfoList";
            AddProperty(info, "Frame Infos", &SequenceFrameInfoList::mValues, "Frame Info");
            AddProperty(info, "WasRLE", &SequenceFrameInfoList::mWasRle);
            AddProperty(
                info,
                "Flags",
                reinterpret_cast<int SequenceFrameInfoList::*>(&SequenceFrameInfoList::mFlags));
            AddEnumProperty(
                info, "BlitType", &SequenceFrameInfoList::mBlitType, GetBlitTypeSerializedValues());
            AddEnumProperty(info,
                            "Blit Type",
                            &SequenceFrameInfoList::mBlitType,
                            GetBlitTypeSerializedValues());
            AddProperty(info, "FPS", &SequenceFrameInfoList::mFramesPerSecond);
        }

        void CopyTo(Sequence& sequence, BlitTypeRevision revision) const;
        const std::vector<FrameInfo*>& GetValues() const;

    private:
        bool mHasTextBlitType;
        std::vector<FrameInfo*> mValues;
        bool mWasRle;
        SequenceFlags mFlags;
        int mBlitType;
        float mFramesPerSecond;
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCE_FRAME_INFO_LIST_H
