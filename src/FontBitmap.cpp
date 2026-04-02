#include "FontBitmap.h"

namespace nuvelocity
{
    FontBitmap::FontBitmap()
            : mFirstAscii(0)
            , mLastAscii(0)
            , mIsFixedWidth(false)
            , mXHeight(0)
    {
    }

    FontBitmap::~FontBitmap() = default;
} // namespace nuvelocity
