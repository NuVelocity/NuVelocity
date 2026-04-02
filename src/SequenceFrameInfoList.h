#ifndef NVE_SEQUENCE_FRAME_INFO_LIST_H
#define NVE_SEQUENCE_FRAME_INFO_LIST_H

#include "BlitTypeConverter.h"
#include "FrameInfo.h"
#include "Sequence.h"
#include "SequenceFlags.h"
#include "model/Model.h"
#include <vector>

namespace nuvelocity
{
    class SequenceFrameInfoList : public Object<SequenceFrameInfoList>
    {
    public:
        SequenceFrameInfoList();
        ~SequenceFrameInfoList();

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CSequenceFrameInfoList";
            AddProperty(aInfo, "Frame Infos", &SequenceFrameInfoList::mValues, "Frame Info");
            AddProperty(aInfo, "WasRLE", &SequenceFrameInfoList::mWasRle);
            AddProperty(aInfo, "Flags", &SequenceFrameInfoList::mFlags);
            AddEnumProperty(aInfo, "BlitType", &SequenceFrameInfoList::mBlitType,
                            GetBlitTypeSerializedValues());
            AddEnumProperty(aInfo, "Blit Type", &SequenceFrameInfoList::mBlitType,
                            GetBlitTypeSerializedValues());
            AddProperty(aInfo, "FPS", &SequenceFrameInfoList::mFramesPerSecond);
        }

        void CopyTo(Sequence& sequence, BlitTypeRevision revision) const;

    private:
        bool mHasTextBlitType;
        std::vector<FrameInfo*> mValues;
        bool mWasRle;
        int mFlags;
        int mBlitType;
        float mFramesPerSecond;
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCE_FRAME_INFO_LIST_H
