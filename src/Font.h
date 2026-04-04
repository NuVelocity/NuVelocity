#ifndef NVE_FONT_H
#define NVE_FONT_H

#include "BlitType.h"
#include "model/Model.h"
#include <string>

namespace nuvelocity
{
    inline constexpr const char* kFontDefaultFamily = "Resources/Fonts/TRUE TYPES/!default.ttf";
    inline constexpr int kFontDefaultPointSize = 12;

    // TODO: check default values.
    class Font : public Object<Font>
    {
    public:
        Font();
        ~Font();

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
    };
} // namespace nuvelocity

#endif // NVE_FONT_H
