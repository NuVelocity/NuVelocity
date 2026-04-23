#include "JWindowAlphaTheme.h"
#include "AlphaSkinBorder.h"
#include "JWindowClassicTheme.h"
#include "system/FontManager.h"
#include "system/Game.h"
#include "system/SpriteBatch.h"
#include "system/ui/Button.h"
#include "system/ui/MdiWindow.h"
#include "system/ui/TextBox.h"
#include "system/ui/WidgetUtils.h"

#include <unordered_map>

namespace nuvelocity
{
    void JWindowAlphaTheme::DrawButton(Game* game, Button* button)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            button == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_FRect rect = button->GetScreenRect();
        SDL_Color textColor = button->GetButtonStyle().textColor;

        // 1. Determine which border to use
        ClassicSkinBorder* border = nullptr;
        bool sunken = false;

        if (!button->IsEnabled())
        {
            border = mOptions->mButtonBorder;
        }
        else if (button->IsPressed())
        {
            border = mOptions->mPressedButtonBorder;
            sunken = true;
        }
        else if (button->IsHovered())
        {
            border = mOptions->mHoverButtonBorder;
        }
        else
        {
            border = mOptions->mButtonBorder;
        }

        // 2. Draw Border and Background
        if (border != nullptr)
        {
            border->Draw(game->mSpriteBatch, rect, sunken);
        }
        else
        {
            // Fallback to flat highlight
            SDL_Color color = mOptions->mBackgroundColor;
            if (button->IsPressed())
            {
                color = mOptions->mSelectColor;
                sunken = true;
            }
            else if (button->IsHovered())
            {
                color = mOptions->mSelectColor;
            }

            WidgetUtils::FillRect(game->mSpriteBatch, rect, color);
            WidgetUtils::DrawRect(game->mSpriteBatch, rect, SDL_Color{100, 100, 100, 255});
        }

        SDL_FRect textRect = rect;
        // Apply slight offset if pressed
        if (button->IsPressed())
        {
            textRect.x += 1;
            textRect.y += 1;
        }

        // Use skin's general font if available
        std::string fontName = mOptions->mGeneralFont.empty() ? "OCR" : mOptions->mGeneralFont;

        game->mFont->DrawStringWithFontAt(fontName,
                                          game->mSpriteBatch,
                                          button->GetDisplayCaption(),
                                          textRect.x + rect.w / 2,
                                          textRect.y + rect.h / 2,
                                          textColor,
                                          13,
                                          TextAlignment::Center,
                                          nullptr,
                                          button->GetMnemonicIndex());

        if (button->GetButtonStyle().showFocusRing && button->IsFocused() && !button->IsHovered())
        {
            const SDL_FRect focusRect{.x = rect.x + 2.0F,
                                      .y = rect.y + 2.0F,
                                      .w = SDL_max(0.0F, rect.w - 4.0F),
                                      .h = SDL_max(0.0F, rect.h - 4.0F)};
            WidgetUtils::DrawRect(game->mSpriteBatch, focusRect, SDL_Color{255, 255, 255, 64});
        }
    }

    void JWindowAlphaTheme::DrawMdiWindow(Game* game, MdiWindow* window)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            window == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_FRect windowRect = window->GetScreenRect();
        const SDL_FRect titleRect = window->GetTitleBarRect();

        // 1. Draw Border and Background
        if (mOptions->mWindowBorder != nullptr)
        {
            mOptions->mWindowBorder->Draw(game->mSpriteBatch, windowRect, GetInnerRect(window));
        }
        else
        {
            WidgetUtils::FillRect(game->mSpriteBatch, windowRect, mOptions->mBackgroundColor);
            WidgetUtils::DrawRect(game->mSpriteBatch, windowRect, SDL_Color{100, 100, 100, 255});
        }

        // 2. Draw Title
        float topFrameHeight = 0.0f;
        if (mOptions->mWindowBorder != nullptr &&
            mOptions->mWindowBorder->mTopCenterHighlightFrame != nullptr)
        {
            if (SDL_Surface* surface =
                    mOptions->mWindowBorder->mTopCenterHighlightFrame->GetSurface())
            {
                topFrameHeight = static_cast<float>(surface->h);
            }
        }

        const float titleX = titleRect.x + (titleRect.w * 0.5f);
        const float titleY =
            titleRect.y + topFrameHeight + static_cast<float>(mOptions->mWindowHeadingOffset);

        // Use skinned font if available
        std::string fontName =
            mOptions->mWindowHeadingFont.empty() ? "Small Blue" : mOptions->mWindowHeadingFont;

        game->mFont->DrawStringWithFontAt(fontName,
                                          game->mSpriteBatch,
                                          window->GetTitle(),
                                          titleX,
                                          titleY,
                                          window->GetWindowStyle().titleTextColor,
                                          13,
                                          TextAlignment::Center);
    }

    void JWindowAlphaTheme::DrawTextBox(Game* game, TextBox* textBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            textBox == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_FRect rect = textBox->GetScreenRect();
        const auto& style = textBox->GetTextBoxStyle();

        const SDL_Color fillColor =
            textBox->IsFocused() ? style.focusedColor : style.unfocusedColor;

        WidgetUtils::FillRect(game->mSpriteBatch, rect, fillColor);

        // TextBox always uses a sunken classic bevel
        WidgetUtils::DrawBevel(
            game->mSpriteBatch,
            rect,
            WidgetUtils::BevelColors{.light = textBox->GetStyle().borderLightColor,
                                     .dark = textBox->GetStyle().borderDarkColor},
            true,
            textBox->GetStyle().borderThickness);

        SDL_FRect textRect{.x = rect.x + 6.0F,
                           .y = rect.y + 3.0F,
                           .w = SDL_max(0.0F, rect.w - 12.0F),
                           .h = SDL_max(0.0F, rect.h - 6.0F)};

        std::string fontName =
            mOptions->mGeneralFont.empty() ? "Small Blue" : mOptions->mGeneralFont;

        game->mFont->DrawStringWithFont(fontName,
                                        game->mSpriteBatch,
                                        textBox->GetText(),
                                        textRect,
                                        style.textColor,
                                        style.fontPointSize,
                                        TextAlignment::Left,
                                        true);

        if (textBox->IsFocused())
        {
            int textWidth = 0;
            int textHeight = 0;
            game->mFont->MeasureStringWithFont(
                fontName, textBox->GetText(), style.fontPointSize, textWidth, textHeight);

            const bool visibleCaret = ((SDL_GetTicks() / 500U) % 2U) == 0U;
            if (visibleCaret)
            {
                SDL_FRect caretRect{.x = rect.x + 6.0F + static_cast<float>(textWidth),
                                    .y = rect.y + 4.0F,
                                    .w = 1.0F,
                                    .h = SDL_max(0.0F, rect.h - 8.0F)};
                WidgetUtils::FillRect(game->mSpriteBatch, caretRect, style.caretColor);
            }
        }
    }

    SDL_FRect JWindowAlphaTheme::GetInnerRect(const MdiWindow* window) const
    {
        // FIXME: hardcoded for now.
        SDL_FRect titleBarRect = window->GetScreenRect();
        return {titleBarRect.x + 20, titleBarRect.y + 30, titleBarRect.w - 40, titleBarRect.h - 50};
    }

    SDL_FRect JWindowAlphaTheme::GetCloseButtonRect(const MdiWindow* window) const
    {
        // FIXME: determine close button rect based on window rect and style.
        // We are currently reusing the metrics for classic theme.
        return window->GetCloseButtonRect();
    }
} // namespace nuvelocity
