#ifndef NVE_FONT_BITMAP_H
#define NVE_FONT_BITMAP_H

#include "Font.h"
#include "Sequence.h"
#include <memory>

namespace nuvelocity
{
    // TODO: check default values.
    class FontBitmap : public Object<FontBitmap, Font>
    {
    public:
        FontBitmap();
        ~FontBitmap();

        int GetFirstAscii() const;
        int GetLastAscii() const;
        bool IsFixedWidth() const;
        int GetXHeight() const;

        void SetFirstAscii(int firstAscii);
        void SetLastAscii(int lastAscii);
        void SetFixedWidth(bool isFixedWidth);
        void SetXHeight(int xHeight);
        void SetSequence(std::unique_ptr<Sequence>&& sequence);

        const Sequence* GetSequence() const;
        Sequence* GetSequence();

        bool IsColorable() const;
        void SetColorable(bool colorable);

        bool MeasureString(const std::string& text,
                           int pointSize,
                           int& width,
                           int& height) const override;
        void DrawString(SpriteBatch* batch,
                        const std::string& text,
                        const SDL_FRect& bounds,
                        const SDL_Color& color,
                        int pointSize,
                        TextAlignment alignment = TextAlignment::Left,
                        bool verticalCenter = true,
                        int underlineIndex = -1) const override;
        void DrawStringAt(SpriteBatch* batch,
                          const std::string& text,
                          float x,
                          float y,
                          const SDL_Color& color,
                          int pointSize,
                          TextAlignment alignment = TextAlignment::Left,
                          bool verticalCenter = false,
                          int underlineIndex = -1) const override;

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFontBitmap";
            AddProperty(aInfo, "First ASCII", &FontBitmap::mFirstAscii);
            AddProperty(aInfo, "Last ASCII", &FontBitmap::mLastAscii);
            AddProperty(aInfo, "Fixed Width", &FontBitmap::mIsFixedWidth);
            AddProperty(aInfo, "Height Without Decenders", &FontBitmap::mXHeight);
            // Extension: allow disabling color modulation for bitmap fonts.
            AddProperty(aInfo, "Colorable", &FontBitmap::mColorable);
        }

    private:
        int mFirstAscii;
        int mLastAscii;
        bool mIsFixedWidth;
        int mXHeight;
        bool mColorable; // NuVelocity extension
        std::unique_ptr<Sequence> mSequence;
    };
} // namespace nuvelocity

#endif // NVE_FONT_BITMAP_H
