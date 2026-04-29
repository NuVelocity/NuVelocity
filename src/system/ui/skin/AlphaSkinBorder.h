#ifndef NVE_ALPHA_SKIN_BORDER_H
#define NVE_ALPHA_SKIN_BORDER_H

#include "model/Object.h"

#include <string>

namespace nuvelocity
{
    class StandAloneFrame;
    class AssetManager;
    class SpriteBatch;

    class AlphaSkinBorder : public Object<AlphaSkinBorder>
    {
    public:
        AlphaSkinBorder();
        virtual ~AlphaSkinBorder();
        std::string mBackgroundTexture;
        std::string mTopLeftAlpha;
        std::string mTopRightAlpha;
        std::string mBottomLeftAlpha;
        std::string mBottomRightAlpha;
        std::string mTopLeftHighlight;
        std::string mTopCenterHighlight;
        std::string mTopRightHighlight;
        std::string mCenterLeftHighlight;
        std::string mCenterRightHighlight;
        std::string mBottomLeftHighlight;
        std::string mBottomCenterHighlight;
        std::string mBottomRightHighlight;
        std::string mSpecialTopRightHighlight;

        StandAloneFrame* mBackgroundFrame = nullptr;
        StandAloneFrame* mTopLeftAlphaFrame = nullptr;
        StandAloneFrame* mTopRightAlphaFrame = nullptr;
        StandAloneFrame* mBottomLeftAlphaFrame = nullptr;
        StandAloneFrame* mBottomRightAlphaFrame = nullptr;
        StandAloneFrame* mTopLeftHighlightFrame = nullptr;
        StandAloneFrame* mTopCenterHighlightFrame = nullptr;
        StandAloneFrame* mTopRightHighlightFrame = nullptr;
        StandAloneFrame* mCenterLeftHighlightFrame = nullptr;
        StandAloneFrame* mCenterRightHighlightFrame = nullptr;
        StandAloneFrame* mBottomLeftHighlightFrame = nullptr;
        StandAloneFrame* mBottomCenterHighlightFrame = nullptr;
        StandAloneFrame* mBottomRightHighlightFrame = nullptr;
        StandAloneFrame* mSpecialTopRightHighlightFrame = nullptr;

        void Load(AssetManager* assets);
        void Draw(SpriteBatch* spriteBatch, const SDL_Rect& windowRect, const SDL_Rect& innerRect);
        void DrawTiledBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect);

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CAlphaSkinBorder";
            AddProperty(info, "Background Texture", &AlphaSkinBorder::mBackgroundTexture);
            AddProperty(info, "Top Left Alpha", &AlphaSkinBorder::mTopLeftAlpha);
            AddProperty(info, "Top Right Alpha", &AlphaSkinBorder::mTopRightAlpha);
            AddProperty(info, "Bottom Left Alpha", &AlphaSkinBorder::mBottomLeftAlpha);
            AddProperty(info, "Bottom Right Alpha", &AlphaSkinBorder::mBottomRightAlpha);
            AddProperty(info, "Top Left Highlight", &AlphaSkinBorder::mTopLeftHighlight);
            AddProperty(info, "Top Center Highlight", &AlphaSkinBorder::mTopCenterHighlight);
            AddProperty(info, "Top Right Highlight", &AlphaSkinBorder::mTopRightHighlight);
            AddProperty(info, "Center Left Highlight", &AlphaSkinBorder::mCenterLeftHighlight);
            AddProperty(info, "Center Right Highlight", &AlphaSkinBorder::mCenterRightHighlight);
            AddProperty(info, "Bottom Left Highlight", &AlphaSkinBorder::mBottomLeftHighlight);
            AddProperty(info, "Bottom Center Highlight", &AlphaSkinBorder::mBottomCenterHighlight);
            AddProperty(info, "Bottom Right Highlight", &AlphaSkinBorder::mBottomRightHighlight);
            AddProperty(
                info, "Special Top Right Highlight", &AlphaSkinBorder::mSpecialTopRightHighlight);
        }

    private:
        void DrawBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect);
        void DrawHighlights(SpriteBatch* spriteBatch, const SDL_Rect& rect) const;
    };
} // namespace nuvelocity

#endif // NVE_ALPHA_SKIN_BORDER_H
