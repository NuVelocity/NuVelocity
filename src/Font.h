#ifndef NVE_FONT_H
#define NVE_FONT_H

#include "BlitType.h"
#include "SpriteBatch.h"
#include "model/Model.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

namespace nuvelocity
{
    enum class TextAlignment
    {
        Left,
        Center,
        Right
    };

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
                                const SDL_FRect& bounds,
                                const SDL_Color& color,
                                int pointSize,
                                TextAlignment alignment = TextAlignment::Left,
                                bool verticalCenter = true,
                                int underlineIndex = -1) const;
        virtual void DrawStringAt(SpriteBatch* batch,
                                  const std::string& text,
                                  float x,
                                  float y,
                                  const SDL_Color& color,
                                  int pointSize,
                                  TextAlignment alignment = TextAlignment::Left,
                                  bool verticalCenter = false,
                                  int underlineIndex = -1) const;

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFont";
            AddProperty(aInfo, "Font Family", &Font::mFontFamily);
            AddEnumProperty<BlitType>(
                aInfo, "Blit Type", &Font::mBlitType, GetBlitTypeSerializedValues());

            // FIXME: colors are not handled by serialization code.
            AddProperty(aInfo, "Generated Color", &Font::mGeneratedColor);

            AddProperty(aInfo, "Point Size", &Font::mPointSize);
            AddProperty(aInfo, "Generate All Caps", &Font::mGenerateAllCaps);
        }

    protected:
        std::string mFontFamily;
        BlitType mBlitType;
        // FIXME: Stored as text because color serialization is not implemented yet.
        std::string mGeneratedColor;
        int mPointSize;
        bool mGenerateAllCaps;

    private:
        TTF_Font* GetTtfFont(int pointSize) const;

        mutable std::unordered_map<int, TTF_Font*> mTtfFonts;
    };
} // namespace nuvelocity

#endif // NVE_FONT_H
