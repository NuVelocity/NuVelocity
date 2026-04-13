#ifndef NVE_ALPHA_SKIN_DATA_H
#define NVE_ALPHA_SKIN_DATA_H

#include "AlphaSkinBorder.h"
#include "ClassicSkinBorder.h"
#include "model/Object.h"

#include <SDL3/SDL.h>
#include <string>

namespace nuvelocity
{
    class AssetManager;
    class AlphaSkinData : public Object<AlphaSkinData>
    {
    public:
        std::string mGeneralFont;
        SDL_Color mShortcutKeyHighlightColor = {0, 0, 0, 255};
        SDL_Color mBackgroundColor = {200, 200, 200, 255};
        SDL_Color mSelectColor = {135, 135, 150, 255};
        std::string mStaticTextControl;
        std::string mWindowHeadingFont;
        int mWindowHeadingOffset = 5;

        AlphaSkinBorder* mWindowBorder = nullptr;
        ClassicSkinBorder* mButtonBorder = nullptr;
        ClassicSkinBorder* mPressedButtonBorder = nullptr;
        ClassicSkinBorder* mHoverButtonBorder = nullptr;

        void Load(AssetManager* assets);

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CAlphaSkinData";
            AddProperty(info, "General Font", &AlphaSkinData::mGeneralFont);
            AddProperty(
                info, "Shortcut Key Highlight Color", &AlphaSkinData::mShortcutKeyHighlightColor);
            AddProperty(info, "Background Color", &AlphaSkinData::mBackgroundColor);
            AddProperty(info, "Select Color", &AlphaSkinData::mSelectColor);
            AddProperty(info, "Static Text Control", &AlphaSkinData::mStaticTextControl);
            AddProperty(info, "Window Heading Font", &AlphaSkinData::mWindowHeadingFont);
            AddProperty(info, "Window Heading Offset", &AlphaSkinData::mWindowHeadingOffset);
            AddProperty(info, "Window Border", &AlphaSkinData::mWindowBorder);
            AddProperty(info, "Button Border", &AlphaSkinData::mButtonBorder);
            AddProperty(info, "Pressed Button Border", &AlphaSkinData::mPressedButtonBorder);
            AddProperty(info, "Hover Button Border", &AlphaSkinData::mHoverButtonBorder);
        }
    };
} // namespace nuvelocity

#endif // NVE_ALPHA_SKIN_DATA_H
