#include "SequenceFrameInfoList.h"

namespace nuvelocity
{
    SequenceFrameInfoList::SequenceFrameInfoList()
            : mHasTextBlitType(false)
            , mWasRle(false)
            , mFlags(0)
            , mBlitType(BLIT_TRANSPARENT_MASK)
            , mFramesPerSecond(kSequenceDefaultFramesPerSecond)
    {
    }

    SequenceFrameInfoList::~SequenceFrameInfoList()
    {
        for (FrameInfo* value : mValues)
        {
            delete value;
        }
        mValues.clear();
    }

    void SequenceFrameInfoList::CopyTo(Sequence& sequence, BlitTypeRevision revision) const
    {
        SequenceFlags flags = static_cast<SequenceFlags>(mFlags);
        sequence.ApplyFrameInfoList(flags, mBlitType, mFramesPerSecond, revision);
    }
} // namespace nuvelocity
