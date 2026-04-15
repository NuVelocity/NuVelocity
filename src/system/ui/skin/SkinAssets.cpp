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

    void AlphaSkinBorder::Draw(SpriteBatch* spriteBatch, const SDL_FRect& rect)
    {
        if (spriteBatch == nullptr)
        {
            return;
        }

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
        SDL_SetSurfaceBlendMode(compositeSurface, SDL_BLENDMODE_BLEND);
        SDL_FRect compositeDest{rect.x,
                                rect.y,
                                static_cast<float>(compositeSurface->w),
                                static_cast<float>(compositeSurface->h)};
        spriteBatch->Draw(compositeSurface, &compositeDest, nullptr);

        // Step 4: Draw highlights on top
        auto drawFrame = [&](StandAloneFrame* frame, float x, float y)
        {
            if (frame != nullptr)
            {
                spriteBatch->Draw(frame, x, y);
            }
        };

        drawFrame(mTopLeftHighlightFrame, rect.x, rect.y);
        drawFrame(mTopRightHighlightFrame, rect.x + static_cast<float>(areaW - rwT), rect.y);
        drawFrame(mBottomLeftHighlightFrame, rect.x, rect.y + static_cast<float>(areaH - bhL));
        drawFrame(mBottomRightHighlightFrame,
                  rect.x + static_cast<float>(areaW - rwB),
                  rect.y + static_cast<float>(areaH - bhR));

        if (mTopCenterHighlightFrame != nullptr && interiorWT > 0)
        {
            WidgetUtils::DrawTiledFrameH(spriteBatch,
                                         mTopCenterHighlightFrame,
                                         {rect.x + static_cast<float>(lwT),
                                          rect.y,
                                          static_cast<float>(interiorWT),
                                          static_cast<float>(thL)});
        }
        if (mBottomCenterHighlightFrame != nullptr && interiorWB > 0)
        {
            WidgetUtils::DrawTiledFrameH(spriteBatch,
                                         mBottomCenterHighlightFrame,
                                         {rect.x + static_cast<float>(lwB),
                                          rect.y + static_cast<float>(areaH - bhR),
                                          static_cast<float>(interiorWB),
                                          static_cast<float>(bhR)});
        }
        if (mCenterLeftHighlightFrame != nullptr && interiorHL > 0)
        {
            WidgetUtils::DrawTiledFrameV(spriteBatch,
                                         mCenterLeftHighlightFrame,
                                         {rect.x,
                                          rect.y + static_cast<float>(thL),
                                          static_cast<float>(lwT),
                                          static_cast<float>(interiorHL)});
        }
        if (mCenterRightHighlightFrame != nullptr && interiorHR > 0)
        {
            WidgetUtils::DrawTiledFrameV(spriteBatch,
                                         mCenterRightHighlightFrame,
                                         {rect.x + static_cast<float>(areaW - rwT),
                                          rect.y + static_cast<float>(thR),
                                          static_cast<float>(rwT),
                                          static_cast<float>(interiorHR)});
        }

        SDL_DestroySurface(compositeSurface);
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
        if (mBackgroundFrame != nullptr)
        {
            WidgetUtils::DrawTiledFrame(spriteBatch, mBackgroundFrame, rect);
        }

        // 2. Draw Bevel
        const float thickness = 1.0F; // Standard classic border thickness
        const SDL_Color topLeft = sunken ? mBottomInnerColor : mTopOuterColor;
        const SDL_Color bottomRight = sunken ? mTopOuterColor : mBottomInnerColor;
        const SDL_Color innerTopLeft = sunken ? mBottomOuterColor : mTopInnerColor;
        const SDL_Color innerBottomRight = sunken ? mTopInnerColor : mBottomOuterColor;

        // Outer Bevel
        WidgetUtils::DrawBevel(spriteBatch,
                               rect,
                               WidgetUtils::BevelColors{.light = topLeft, .dark = bottomRight},
                               false,
                               thickness);

        // Inner Bevel (if margin allows)
        if (mTextureMargin > 1)
        {
            SDL_FRect innerRect = rect;
            innerRect.x += thickness;
            innerRect.y += thickness;
            innerRect.w -= thickness * 2.0F;
            innerRect.h -= thickness * 2.0F;
            WidgetUtils::DrawBevel(
                spriteBatch,
                innerRect,
                WidgetUtils::BevelColors{.light = innerTopLeft, .dark = innerBottomRight},
                false,
                thickness);
        }
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
