#include "SequenceFrameInfoList.h"

namespace nuvelocity
{
    SequenceFrameInfoList::SequenceFrameInfoList()
            : mHasTextBlitType(false)
            , mWasRle(false)
            , mFlags(static_cast<SequenceFlags>(0))
            , mBlitType(BlitTransparentMask)
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
        sequence.ApplyFrameInfoList(mFlags, mBlitType, mFramesPerSecond, revision);
    }

    const std::vector<FrameInfo*>& SequenceFrameInfoList::GetValues() const
    {
        return mValues;
    }
} // namespace nuvelocity
