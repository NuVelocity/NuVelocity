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

    inline void ApplyAlphaMask(StandAloneFrame* frame,
                               int startX,
                               int startY,
                               int areaW,
                               int areaH,
                               SDL_Surface* compositeSurface,
                               const SDL_PixelFormatDetails* dstDetails)
    {
        if (frame == nullptr)
        {
            return;
        }
        SDL_Surface* rawMask = frame->GetSurface();
        if (rawMask == nullptr)
        {
            return;
        }

        // Convert mask to match our composite format.
        SDL_Surface* mask = SDL_ConvertSurface(rawMask, compositeSurface->format);
        if (mask == nullptr)
        {
            return;
        }

        if (SDL_MUSTLOCK(compositeSurface))
        {
            SDL_LockSurface(compositeSurface);
        }
        if (SDL_MUSTLOCK(mask))
        {
            SDL_LockSurface(mask);
        }

        Uint32* dstPixels = static_cast<Uint32*>(compositeSurface->pixels);
        Uint32* maskPixels = static_cast<Uint32*>(mask->pixels);

        // Pitch is in bytes; divide by 4 to get the pitch in 32-bit words
        int dstPitch = compositeSurface->pitch / 4;
        int maskPitch = mask->pitch / 4;

        const SDL_PixelFormatDetails* maskDetails = SDL_GetPixelFormatDetails(mask->format);

        for (int my = 0; my < mask->h; ++my)
        {
            int dy = startY + my;
            if (dy < 0 || dy >= areaH)
            {
                continue; // Keep within bounds
            }
            for (int mx = 0; mx < mask->w; ++mx)
            {
                int dx = startX + mx;
                if (dx < 0 || dx >= areaW)
                {
                    continue; // Keep within bounds
                }
                // Extract mask Alpha
                Uint32 mPixel = maskPixels[my * maskPitch + mx];
                Uint8 mR, mG, mB, mA;
                SDL_GetRGBA(mPixel, maskDetails, nullptr, &mR, &mG, &mB, &mA);

                // Extract destination RGB
                int dstIndex = dy * dstPitch + dx;
                Uint32 dPixel = dstPixels[dstIndex];
                Uint8 dR, dG, dB, dA;
                SDL_GetRGBA(dPixel, dstDetails, nullptr, &dR, &dG, &dB, &dA);

                // Reconstruct pixel: Keep destination RGB, overwrite with mask Alpha
                dstPixels[dstIndex] = SDL_MapRGBA(dstDetails, nullptr, dR, dG, dB, mA);
            }
        }

        if (SDL_MUSTLOCK(mask))
        {
            SDL_UnlockSurface(mask);
        }
        if (SDL_MUSTLOCK(compositeSurface))
        {
            SDL_UnlockSurface(compositeSurface);
        }

        SDL_DestroySurface(mask);
    }

    void AlphaSkinBorder::DrawBackground(SpriteBatch* spriteBatch, const SDL_Rect& rect)
    {
        if (rect.w <= 0 || rect.h <= 0)
        {
            return;
        }

        const int areaW = rect.w;
        const int areaH = rect.h;

        SDL_Surface* compositeSurface = SDL_CreateSurface(areaW, areaH, SDL_PIXELFORMAT_RGBA32);
        if (compositeSurface == nullptr)
        {
            return;
        }

        // Step 1: Tile the background across the full composite surface
        if (mBackgroundFrame != nullptr)
        {
            SDL_Surface* bgSurface = mBackgroundFrame->GetSurface();
            if (bgSurface != nullptr)
            {
                SDL_SetSurfaceBlendMode(bgSurface, SDL_BLENDMODE_NONE);
                for (int y = 0; y < areaH; y += bgSurface->h)
                {
                    for (int x = 0; x < areaW; x += bgSurface->w)
                    {
                        SDL_Rect dstRect{.x = x, .y = y, .w = bgSurface->w, .h = bgSurface->h};
                        SDL_BlitSurface(bgSurface, nullptr, compositeSurface, &dstRect);
                    }
                }
            }
        }

        // Step 2: Manually stamp corner alpha masks using the inline function
        const SDL_PixelFormatDetails* dstDetails =
            SDL_GetPixelFormatDetails(compositeSurface->format);

        // Get dimensions for positioning corners correctly
        const int rwT = (mTopRightAlphaFrame != nullptr) ? mTopRightAlphaFrame->GetWidth() : 0;
        const int rwB =
            (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetWidth() : 0;
        const int bhL = (mBottomLeftAlphaFrame != nullptr) ? mBottomLeftAlphaFrame->GetHeight() : 0;
        const int bhR =
            (mBottomRightAlphaFrame != nullptr) ? mBottomRightAlphaFrame->GetHeight() : 0;

        // Apply Corners
        ApplyAlphaMask(mTopLeftAlphaFrame, 0, 0, areaW, areaH, compositeSurface, dstDetails);
        ApplyAlphaMask(
            mTopRightAlphaFrame, areaW - rwT, 0, areaW, areaH, compositeSurface, dstDetails);
        ApplyAlphaMask(
            mBottomLeftAlphaFrame, 0, areaH - bhL, areaW, areaH, compositeSurface, dstDetails);
        ApplyAlphaMask(mBottomRightAlphaFrame,
                       areaW - rwB,
                       areaH - bhR,
                       areaW,
                       areaH,
                       compositeSurface,
                       dstDetails);

        // Step 3: Draw the composite surface to the sprite batch
        SDL_SetSurfaceBlendMode(compositeSurface, SDL_BLENDMODE_BLEND);
        SDL_Rect compositeDest{.x = rect.x, .y = rect.y, .w = areaW, .h = areaH};
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

        WidgetUtils::DrawBevel(spriteBatch, rect, borderColors, mTextureMargin);
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
