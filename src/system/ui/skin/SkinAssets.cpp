#include "AlphaSkinBorder.h"
#include "AlphaSkinData.h"
#include "ClassicSkinBorder.h"
#include "Colors.h"
#include "JWindowAlphaTheme.h"
#include <StandAloneFrame.h>
#include <system/AssetManager.h>
#include <system/SpriteBatch.h>
#include <system/ui/WidgetUtils.h>

namespace nuvelocity
{
    AlphaSkinBorder::AlphaSkinBorder() = default;

    AlphaSkinBorder::~AlphaSkinBorder() = default;

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

    void AlphaSkinBorder::DrawBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect)
    {
        if (rect.w <= 0 || rect.h <= 0)
        {
            return;
        }

        // Calculate individual corner dimensions as integers for stable mask assembly.
        // We use separate metrics for each side to handle asymmetrical skin assets.
        int lwT = (mTopLeftAlphaFrame != nullptr) ? mTopLeftAlphaFrame->GetWidth() : 0;
        int lwB = (mBottomLeftAlphaFrame != nullptr) ? mBottomLeftAlphaFrame->GetWidth() : 0;
        int rwT = (mTopRightAlphaFrame != nullptr) ? mTopRightAlphaFrame->GetWidth() : 0;
        int rwB = (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetWidth() : 0;

        int thL = (mTopLeftAlphaFrame != nullptr) ? mTopLeftAlphaFrame->GetHeight() : 0;
        int thR = (mTopRightAlphaFrame != nullptr) ? mTopRightAlphaFrame->GetHeight() : 0;
        int bhL = (mBottomLeftAlphaFrame != nullptr) ? mBottomLeftAlphaFrame->GetHeight() : 0;
        int bhR = (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetHeight() : 0;

        int areaW = rect.w;
        int areaH = rect.h;

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
            SDL_Rect interiorRect{.x = fillX, .y = fillY, .w = fillW, .h = fillH};
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
                    SDL_Rect dstRect{.x = x, .y = y, .w = s->w, .h = s->h};
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
            SDL_Rect srcRect{.x = s->w - 1, .y = 0, .w = 1, .h = s->h};
            SDL_Rect dstRect{.x = lwT, .y = 0, .w = interiorWT, .h = s->h};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }
        if (interiorWB > 0 && mBottomRightAlphaFrame != nullptr)
        {
            SDL_Surface* s = mBottomRightAlphaFrame->GetSurface();
            SDL_Rect srcRect{.x = 0, .y = 0, .w = 1, .h = s->h};
            SDL_Rect dstRect{.x = lwB, .y = areaH - bhR, .w = interiorWB, .h = s->h};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }

        // Vertical edges
        if (interiorHL > 0 && mTopLeftAlphaFrame != nullptr)
        {
            SDL_Surface* s = mTopLeftAlphaFrame->GetSurface();
            SDL_Rect srcRect{.x = 0, .y = s->h - 1, .w = s->w, .h = 1};
            SDL_Rect dstRect{.x = 0, .y = thL, .w = s->w, .h = interiorHL};
            SDL_BlitSurfaceScaled(s, &srcRect, compositeSurface, &dstRect, SDL_SCALEMODE_NEAREST);
        }
        if (interiorHR > 0 && mTopRightAlphaFrame != nullptr)
        {
            SDL_Surface* s = mTopRightAlphaFrame->GetSurface();
            SDL_Rect srcRect{.x = 0, .y = s->h - 1, .w = s->w, .h = 1};
            SDL_Rect dstRect{.x = areaW - rwT, .y = thR, .w = s->w, .h = interiorHR};
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
                        SDL_Rect dstRect{.x = x, .y = y, .w = bgSurface->w, .h = bgSurface->h};
                        SDL_BlitSurface(bgSurface, nullptr, compositeSurface, &dstRect);
                    }
                }
            }
        }

        // Step 3: Draw the composite surface to the sprite batch
        SDL_SetSurfaceBlendMode(compositeSurface, SDL_BLENDMODE_BLEND);

        SDL_Rect compositeDest{.x = rect.x, .y = rect.y, .w = rect.w, .h = rect.h};
        spriteBatch->Draw(compositeSurface, &compositeDest, nullptr);
        SDL_DestroySurface(compositeSurface);
    }

    void AlphaSkinBorder::DrawTiledBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect)
    {
        if (mBackgroundFrame == nullptr)
        {
            return;
        }
        SDL_Surface* surface = mBackgroundFrame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
        WidgetUtils::DrawTiledFrame(spriteBatch, mBackgroundFrame, rect);
    }

    void AlphaSkinBorder::DrawHighlights(SpriteBatch* spriteBatch, const SDL_Rect& rect) const
    {
        // Step 4: Draw highlights on top
        int areaW = rect.w;
        int areaH = rect.h;
        auto drawFrame = [&](StandAloneFrame* frame, int x, int y)
        {
            if (frame != nullptr)
            {
                spriteBatch->Draw(frame, x, y, Colors::White, false);
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
        drawFrame(mTopLeftHighlightFrame, rect.x, rect.y);
        drawFrame(mTopRightHighlightFrame, rect.x + areaW - hlwTR, rect.y);
        drawFrame(mBottomLeftHighlightFrame, rect.x, rect.y + areaH - hlhBL);
        drawFrame(mBottomRightHighlightFrame, rect.x + areaW - hlwBR, rect.y + areaH - hlhBR);

        // Top center highlight
        if (mTopCenterHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameH(
                spriteBatch,
                mTopCenterHighlightFrame,
                {.x = rect.x + hlwTR, .y = rect.y, .w = areaW - hlwTL - hlwTR, .h = hlhTC});
        }
        // Bottom center highlight
        if (mBottomCenterHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameH(spriteBatch,
                                         mBottomCenterHighlightFrame,
                                         {.x = rect.x + hlwBL,
                                          .y = rect.y + areaH - hlhBC,
                                          .w = areaW - hlwBL - hlwBR,
                                          .h = hlhBC});
        }
        // Center left highlight
        if (mCenterLeftHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameV(
                spriteBatch,
                mCenterLeftHighlightFrame,
                {.x = rect.x, .y = rect.y + hlhTR, .w = hlwCL, .h = areaH - hlhTR - hlhBR});
        }
        // Center right highlight
        if (mCenterRightHighlightFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrameV(spriteBatch,
                                         mCenterRightHighlightFrame,
                                         {.x = rect.x + areaW - hlwCR,
                                          .y = rect.y + hlhTR,
                                          .w = hlwCR,
                                          .h = areaH - hlhTR - hlhBR});
        }
    }

    void AlphaSkinBorder::Draw(SpriteBatch* spriteBatch,
                               const SDL_Rect& windowRect,
                               const SDL_Rect& innerRect)
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

    void ClassicSkinBorder::DrawBorder(SpriteBatch* spriteBatch, const SDL_Rect& rect, bool sunken)
    {
        if (spriteBatch == nullptr)
        {
            return;
        }

        auto borderColors = sunken
                                ? WidgetUtils::BorderColors{.topLeftOuter = mTopOuterColor,
                                                            .topLeftInner = mTopInnerColor,
                                                            .bottomRightInner = mBottomInnerColor,
                                                            .bottomRightOuter = mBottomOuterColor}
                                : WidgetUtils::BorderColors{.topLeftOuter = mBottomInnerColor,
                                                            .topLeftInner = mBottomOuterColor,
                                                            .bottomRightInner = mTopOuterColor,
                                                            .bottomRightOuter = mTopInnerColor};

        WidgetUtils::DrawBorder(spriteBatch, rect, borderColors, mTextureMargin);
    }

    void ClassicSkinBorder::DrawBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect)
    {
        if (spriteBatch == nullptr)
        {
            return;
        }

        SDL_Rect bgRect = {.x = rect.x + mTextureMargin,
                           .y = rect.y + mTextureMargin,
                           .w = SDL_max(0, rect.w - (mTextureMargin * 2)),
                           .h = SDL_max(0, rect.h - (mTextureMargin * 2))};
        if (mBackgroundFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrame(spriteBatch, mBackgroundFrame, bgRect);
        }
    }

    void AlphaSkinData::Load(AssetManager* assets) const
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
