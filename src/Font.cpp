#include "Font.h"

namespace nuvelocity
{
    Font::Font()
            : mFontFamily(kFontDefaultFamily)
            , mBlitType(BLIT_TRANSPARENT_MASK)
            , mPointSize(kFontDefaultPointSize)
            , mGenerateAllCaps(false)
    {
    }

    Font::~Font() = default;
} // namespace nuvelocity
