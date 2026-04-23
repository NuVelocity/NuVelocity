#include "AlphaSkinBorder.h"
#include "AlphaSkinData.h"
#include "ClassicSkinBorder.h"
#include "JWindowAlphaTheme.h"
#include <StandAloneFrame.h>
#include <system/AssetManager.h>
#include <system/SpriteBatch.h>
#include <system/ui/WidgetUtils.h>

namespace nuvelocity
{
    void AlphaSkinBorder::Load(AssetManager* assets)
    {
        if (assets == nullptr)
        {
            return;
        }

        auto load = [&](const std::string& path) -> StandAloneFrame*
        {
            if (path.empty())
            {
                return nullptr;
            }
            return assets->LoadStandAloneFrame("Resources/" + path);
        };

        mBackgroundFrame = load(mBackgroundTexture);
        mTopLeftAlphaFrame = load(mTopLeftAlpha);
        mTopRightAlphaFrame = load(mTopRightAlpha);
        mBottomLeftAlphaFrame = load(mBottomLeftAlpha);
        mBottomRightAlphaFrame = load(mBottomRightAlpha);
        mTopLeftHighlightFrame = load(mTopLeftHighlight);
        mTopCenterHighlightFrame = load(mTopCenterHighlight);
        mTopRightHighlightFrame = load(mTopRightHighlight);
        mCenterLeftHighlightFrame = load(mCenterLeftHighlight);
        mCenterRightHighlightFrame = load(mCenterRightHighlight);
        mBottomLeftHighlightFrame = load(mBottomLeftHighlight);
        mBottomCenterHighlightFrame = load(mBottomCenterHighlight);
        mBottomRightHighlightFrame = load(mBottomRightHighlight);
        mSpecialTopRightHighlightFrame = load(mSpecialTopRightHighlight);
    }

    void AlphaSkinBorder::DrawBackground(SpriteBatch* spriteBatch, const SDL_FRect& rect)
    {
        // 1. Calculate individual corner dimensions as integers for stable mask assembly.
        // We use separate metrics for each side to handle asymmetrical skin assets.
        int lwT = (mTopLeftAlphaFrame != nullptr) ? mTopLeftAlphaFrame->GetWidth() : 0;
        int lwB = (mBottomLeftAlphaFrame != nullptr) ? mBottomLeftAlphaFrame->GetWidth() : 0;
        int rwT = (mTopRightAlphaFrame != nullptr) ? mTopRightAlphaFrame->GetWidth() : 0;
        int rwB = (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetWidth() : 0;

        int thL = (mTopLeftAlphaFrame != nullptr) ? mTopLeftAlphaFrame->GetHeight() : 0;
        int thR = (mTopRightAlphaFrame != nullptr) ? mTopRightAlphaFrame->GetHeight() : 0;
        int bhL = (mBottomLeftAlphaFrame != nullptr) ? mBottomLeftAlphaFrame->GetHeight() : 0;
        int bhR = (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetHeight() : 0;

        int areaW = static_cast<int>(rect.w);
        int areaH = static_cast<int>(rect.h);

        int interiorWT = SDL_max(0, areaW - lwT - rwT);
        int interiorWB = SDL_max(0, areaW - lwB - rwB);
        int interiorHL = SDL_max(0, areaH - thL - bhL);
        int interiorHR = SDL_max(0, areaH - thR - bhR);

        // Create an intermediary surface for compositing
        SDL_Surface* compositeSurface = SDL_CreateSurface(areaW, areaH, SDL_PIXELFORMAT_RGBA32);
        if (compositeSurface == nullptr)
        {
            return;
        }

        // Fill composite surface with transparent black
        SDL_FillSurfaceRect(compositeSurface, nullptr, 0x00000000);

        // Fill the interior area with opaque white (Alpha 255).
        // To be safe, we use the smallest corner dimensions to ensure a slight overlap with edges.
        int fillX = SDL_min(lwT, lwB);
        int fillY = SDL_min(thL, thR);
        int fillW = areaW - fillX - SDL_min(rwT, rwB);
        int fillH = areaH - fillY - SDL_min(bhL, bhR);

        if (fillW > 0 && fillH > 0)
        {
            SDL_Rect interiorRect{fillX, fillY, fillW, fillH};
            SDL_FillSurfaceRect(compositeSurface, &interiorRect, 0xFFFFFFFF);
        }

        // Step 1: Assemble the Alpha Mask onto the composite surface
        auto blitAlpha = [&](StandAloneFrame* frame, int x, int y)
        {
            if (frame != nullptr)
            {
                SDL_Surface* s = frame->GetSurface();
                if (s != nullptr)
                {
                    SDL_Rect dstRect{x, y, s->w, s->h};
                    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
                    SDL_BlitSurface(s, nullptr, compositeSurface, &dstRect);
                }
            }
        };

        // Corners
        blitAlpha(mTopLeftAlphaFrame, 0, 0);
        blitAlpha(mTopRightAlphaFrame, areaW - rwT, 0);
        blitAlpha(mBottomLeftAlphaFrame, 0, areaH - bhL);
        blitAlpha(mBottomRightAlphaFrame, areaW - rwB, areaH - bhR);

        // Stretched Edges (using 1-pixel slices from corner masks)

        // Horizontal edges
        if (interiorWT > 0 && mTopLeftAlphaFrame != nullptr)
        {
            SDL_Surface* s = mTopLeftAlphaFrame->GetSurface();
            SDL_Rect srcRect{s->w - 1, 0, 1, s->h};
            SDL_Rect dstRect{lwT, 0, interiorWT, s->h};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }
        if (interiorWB > 0 && mBottomRightAlphaFrame != nullptr)
        {
            SDL_Surface* s = mBottomRightAlphaFrame->GetSurface();
            SDL_Rect srcRect{0, 0, 1, s->h};
            SDL_Rect dstRect{lwB, areaH - bhR, interiorWB, s->h};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }

        // Vertical edges
        if (interiorHL > 0 && mTopLeftAlphaFrame != nullptr)
        {
            SDL_Surface* s = mTopLeftAlphaFrame->GetSurface();
            SDL_Rect srcRect{0, s->h - 1, s->w, 1};
            SDL_Rect dstRect{0, thL, s->w, interiorHL};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }
        if (interiorHR > 0 && mTopRightAlphaFrame != nullptr)
        {
            SDL_Surface* s = mTopRightAlphaFrame->GetSurface();
            SDL_Rect srcRect{0, s->h - 1, s->w, 1};
            SDL_Rect dstRect{areaW - rwT, thR, s->w, interiorHR};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }

        // Step 2: Tile background texture using modulation to apply the mask
        if (mBackgroundFrame != nullptr)
        {
            SDL_Surface* bgSurface = mBackgroundFrame->GetSurface();
            if (bgSurface != nullptr)
            {
                SDL_SetSurfaceBlendMode(bgSurface, SDL_BLENDMODE_MOD);
                for (int y = 0; y < compositeSurface->h; y += bgSurface->h)
                {
                    for (int x = 0; x < compositeSurface->w; x += bgSurface->w)
                    {
                        SDL_Rect dstRect{x, y, bgSurface->w, bgSurface->h};
                        SDL_BlitSurface(bgSurface, nullptr, compositeSurface, &dstRect);
                    }
                }
            }
        }

        // Step 3: Draw the composite surface to the sprite batch
        float alphaRectX = rect.x;
        float alphaRectY = rect.y;
        SDL_SetSurfaceBlendMode(compositeSurface, SDL_BLENDMODE_BLEND);
        SDL_FRect compositeDest{alphaRectX,
                                alphaRectY,
                                static_cast<float>(compositeSurface->w),
                                static_cast<float>(compositeSurface->h)};
        spriteBatch->Draw(compositeSurface, &compositeDest, nullptr);

        SDL_DestroySurface(compositeSurface);
    }

    void AlphaSkinBorder::DrawHighlights(SpriteBatch* spriteBatch, const SDL_FRect& rect)
    {
        // Step 4: Draw highlights on top
        float highlightRectX = rect.x;
        float highlightRectY = rect.y;
        int areaW = static_cast<int>(rect.w);
        int areaH = static_cast<int>(rect.h);
        auto drawFrame = [&](StandAloneFrame* frame, float x, float y)
        {
            if (frame != nullptr)
            {
                spriteBatch->Draw(frame, x, y);
            }
        };

        // Use common vars for highlight frame dimensions, matching alpha naming style
        int hlwTL = (mTopLeftHighlightFrame != nullptr) ? mTopLeftHighlightFrame->GetWidth() : 0;
        int hlwTR = (mTopRightHighlightFrame != nullptr) ? mTopRightHighlightFrame->GetWidth() : 0;
        int hlwBL =
            (mBottomLeftHighlightFrame != nullptr) ? mBottomLeftHighlightFrame->GetWidth() : 0;
        int hlwBR =
            (mBottomRightHighlightFrame != nullptr) ? mBottomRightHighlightFrame->GetWidth() : 0;
        int hlwCL =
            (mCenterLeftHighlightFrame != nullptr) ? mCenterLeftHighlightFrame->GetWidth() : 0;
        int hlwCR =
            (mCenterRightHighlightFrame != nullptr) ? mCenterRightHighlightFrame->GetWidth() : 0;

        int hlhTL = (mTopLeftHighlightFrame != nullptr) ? mTopLeftHighlightFrame->GetHeight() : 0;
        int hlhTR = (mTopRightHighlightFrame != nullptr) ? mTopRightHighlightFrame->GetHeight() : 0;
        int hlhBL =
            (mBottomLeftHighlightFrame != nullptr) ? mBottomLeftHighlightFrame->GetHeight() : 0;
        int hlhBR =
            (mBottomRightHighlightFrame != nullptr) ? mBottomRightHighlightFrame->GetHeight() : 0;
        int hlhTC =
            (mTopCenterHighlightFrame != nullptr) ? mTopCenterHighlightFrame->GetHeight() : 0;
        int hlhBC =
            (mBottomCenterHighlightFrame != nullptr) ? mBottomCenterHighlightFrame->GetHeight() : 0;
        int hlhCL =
            (mCenterLeftHighlightFrame != nullptr) ? mCenterLeftHighlightFrame->GetHeight() : 0;
        int hlhCR =
            (mCenterRightHighlightFrame != nullptr) ? mCenterRightHighlightFrame->GetHeight() : 0;

        // Corners
        drawFrame(mTopLeftHighlightFrame, highlightRectX, highlightRectY);
        drawFrame(mTopRightHighlightFrame,
                  highlightRectX + static_cast<float>(areaW - hlwTR),
                  highlightRectY);
        drawFrame(mBottomLeftHighlightFrame,
                  highlightRectX,
                  highlightRectY + static_cast<float>(areaH - hlhBL));
        drawFrame(mBottomRightHighlightFrame,
                  highlightRectX + static_cast<float>(areaW - hlwBR),
                  highlightRectY + static_cast<float>(areaH - hlhBR));

        // Top center highlight
        if (mTopCenterHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameH(spriteBatch,
                                         mTopCenterHighlightFrame,
                                         {highlightRectX + static_cast<float>(hlwTR),
                                          highlightRectY,
                                          static_cast<float>(areaW - hlwTL - hlwTR),
                                          static_cast<float>(hlhTC)});
        }
        // Bottom center highlight
        if (mBottomCenterHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameH(spriteBatch,
                                         mBottomCenterHighlightFrame,
                                         {highlightRectX + static_cast<float>(hlwBL),
                                          highlightRectY + static_cast<float>(areaH - hlhBC),
                                          static_cast<float>(areaW - hlwBL - hlwBR),
                                          static_cast<float>(hlhBC)});
        }
        // Center left highlight
        if (mCenterLeftHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameV(spriteBatch,
                                         mCenterLeftHighlightFrame,
                                         {highlightRectX,
                                          highlightRectY + static_cast<float>(hlhTR),
                                          static_cast<float>(hlwCL),
                                          static_cast<float>(areaH - hlhTR - hlhBR)});
        }
        // Center right highlight
        if (mCenterRightHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameV(spriteBatch,
                                         mCenterRightHighlightFrame,
                                         {highlightRectX + static_cast<float>(areaW - hlwCR),
                                          highlightRectY + static_cast<float>(hlhTR),
                                          static_cast<float>(hlwCR),
                                          static_cast<float>(areaH - hlhTR - hlhBR)});
        }
    }

    void AlphaSkinBorder::Draw(SpriteBatch* spriteBatch,
                               const SDL_FRect& windowRect,
                               const SDL_FRect& innerRect)
    {
        if (spriteBatch == nullptr)
        {
            return;
        }

        DrawBackground(spriteBatch, innerRect);
        DrawHighlights(spriteBatch, windowRect);
    }

    void ClassicSkinBorder::Load(AssetManager* assets)
    {
        if (assets == nullptr)
        {
            return;
        }
        if (!mBackgroundTexture.empty())
        {
            mBackgroundFrame = assets->LoadStandAloneFrame("Resources/" + mBackgroundTexture);
        }
    }

    void ClassicSkinBorder::Draw(SpriteBatch* spriteBatch, const SDL_FRect& rect, bool sunken)
    {
        if (spriteBatch == nullptr)
        {
            return;
        }

        // 1. Draw Background (Tiled)
        SDL_FRect bgRect = {.x = rect.x + mTextureMargin,
                            .y = rect.y + mTextureMargin,
                            .w = SDL_max(0.0F, rect.w - (mTextureMargin * 2)),
                            .h = SDL_max(0.0F, rect.h - (mTextureMargin * 2))};
        if (mBackgroundFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrame(spriteBatch, mBackgroundFrame, bgRect);
        }

        // 2. Draw Border
        WidgetUtils::DrawBorder(spriteBatch,
                                rect,
                                WidgetUtils::BorderColors{.topLeftOuter = mTopOuterColor,
                                                          .topLeftInner = mTopInnerColor,
                                                          .bottomRightInner = mBottomInnerColor,
                                                          .bottomRightOuter = mBottomOuterColor},
                                mTextureMargin);
    }

    void AlphaSkinData::Load(AssetManager* assets)
    {
        if (assets == nullptr)
        {
            return;
        }
        if (mWindowBorder != nullptr)
        {
            mWindowBorder->Load(assets);
        }
        if (mButtonBorder != nullptr)
        {
            mButtonBorder->Load(assets);
        }
        if (mPressedButtonBorder != nullptr)
        {
            mPressedButtonBorder->Load(assets);
        }
        if (mHoverButtonBorder != nullptr)
        {
            mHoverButtonBorder->Load(assets);
        }
    }

    void JWindowAlphaTheme::Load(AssetManager* assets)
    {
        if (mOptions != nullptr)
        {
            mOptions->Load(assets);
        }
    }
} // namespace nuvelocity
