#ifndef NVE_FONT_BITMAP_H
#define NVE_FONT_BITMAP_H

#include "Font.h"

namespace nuvelocity
{
    // TODO: check default values.
    class FontBitmap : public Object<FontBitmap, Font>
    {
    public:
        FontBitmap();
        ~FontBitmap();

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFontBitmap";
            AddProperty(aInfo, "First ASCII", &FontBitmap::mFirstAscii);
            AddProperty(aInfo, "Last ASCII", &FontBitmap::mLastAscii);
            AddProperty(aInfo, "Fixed Width", &FontBitmap::mIsFixedWidth);
            AddProperty(aInfo, "Height Without Decenders", &FontBitmap::mXHeight);
        }

    private:
        int mFirstAscii;
        int mLastAscii;
        bool mIsFixedWidth;
        int mXHeight;
    };
} // namespace nuvelocity

#endif // NVE_FONT_BITMAP_H
