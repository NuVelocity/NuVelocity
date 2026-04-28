#ifndef NVE_CLASSIC_SKIN_BORDER_H
#define NVE_CLASSIC_SKIN_BORDER_H

#include "model/Object.h"

#include <SDL3/SDL.h>
#include <string>

namespace nuvelocity
{
    class StandAloneFrame;
    class AssetManager;
    class SpriteBatch;

    class ClassicSkinBorder : public Object<ClassicSkinBorder>
    {
    public:
        std::string mBackgroundTexture;
        int mTextureMargin = 1;
        SDL_Color mTopOuterColor = {128, 128, 200, 60};
        SDL_Color mTopInnerColor = {128, 128, 200, 60};
        SDL_Color mBottomInnerColor = {0, 0, 0, 128};
        SDL_Color mBottomOuterColor = {0, 0, 0, 128};

        StandAloneFrame* mBackgroundFrame = nullptr;

        void Load(AssetManager* assets);
        void DrawBorder(SpriteBatch* spriteBatch, const SDL_Rect& rect, bool sunken = true);
        void DrawBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect);

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CClassicSkinBorder";
            AddProperty(info, "Background Texture", &ClassicSkinBorder::mBackgroundTexture);
            AddProperty(info, "Texture Margin", &ClassicSkinBorder::mTextureMargin);
            AddProperty(info, "Top Outer Color", &ClassicSkinBorder::mTopOuterColor);
            AddProperty(info, "Top Inner Color", &ClassicSkinBorder::mTopInnerColor);
            AddProperty(info, "Bottom Inner Color", &ClassicSkinBorder::mBottomInnerColor);
            AddProperty(info, "Bottom Outer Color", &ClassicSkinBorder::mBottomOuterColor);
        }
    };
} // namespace nuvelocity

#endif // NVE_CLASSIC_SKIN_BORDER_H
