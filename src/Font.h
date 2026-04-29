#ifndef NVE_FONT_H
#define NVE_FONT_H

#include "BlitType.h"
#include "Object.h"
#include "SpriteBatch.h"
#include "TextAlignment.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

namespace nuvelocity
{
    inline constexpr const char* kFontDefaultFamily = "Resources/Fonts/TRUE TYPES/!default.ttf";
    inline constexpr int kFontDefaultPointSize = 12;

    // TODO: check default values.
    class Font : public Object<Font>
    {
    public:
        Font();
        virtual ~Font();

        virtual bool
        MeasureString(const std::string& text, int pointSize, int& width, int& height) const;
        virtual void DrawString(SpriteBatch* batch,
                                const std::string& text,
                                const SDL_Rect& bounds,
                                const SDL_Color& color,
                                int pointSize,
                                TextAlignment alignment = TextAlignment::Left,
                                bool verticalCenter = true,
                                int underlineIndex = -1,
                                const SDL_Color& underlineColor = Colors::White) const;
        virtual void DrawStringAt(SpriteBatch* batch,
                                  const std::string& text,
                                  int x,
                                  int y,
                                  const SDL_Color& color,
                                  int pointSize,
                                  TextAlignment alignment = TextAlignment::Left,
                                  bool verticalCenter = false,
                                  int underlineIndex = -1,
                                  const SDL_Color& underlineColor = Colors::White) const;

        void AttachFontStream(SDL_IOStream* fontStream)
        {
            mFontStream = fontStream;
        }

        std::string GetFontFamily() const
        {
            return mFontFamily;
        }

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CFont";
            AddProperty(info, "Font Family", &Font::mFontFamily);
            AddEnumProperty<BlitType>(
                info, "Blit Type", &Font::mBlitType, GetBlitTypeSerializedValues());

            // FIXME: colors are not handled by serialization code.
            AddProperty(info, "Generated Color", &Font::mGeneratedColor);

            AddProperty(info, "Point Size", &Font::mPointSize);
            AddProperty(info, "Generate All Caps", &Font::mGenerateAllCaps);
        }

    protected:
        std::string mFontFamily;
        BlitType mBlitType;
        // FIXME: Stored as text because color serialization is not implemented yet.
        std::string mGeneratedColor;
        int mPointSize;
        bool mGenerateAllCaps;

    private:
        SDL_IOStream* mFontStream;
        TTF_Font* GetTtfFont(int pointSize) const;

        mutable std::unordered_map<int, TTF_Font*> mTtfFonts;
    };
} // namespace nuvelocity

#endif // NVE_FONT_H
