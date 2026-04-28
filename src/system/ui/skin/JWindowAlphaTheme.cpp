#include "JWindowAlphaTheme.h"
#include "AlphaSkinBorder.h"
#include "system/FontManager.h"
#include "system/Game.h"
#include "system/ui/Button.h"
#include "system/ui/MdiWindow.h"
#include "system/ui/TextBox.h"
#include "system/ui/WidgetUtils.h"

namespace nuvelocity
{
    void JWindowAlphaTheme::DrawButton(Game* game, Button* button)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            button == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_Rect rect = button->GetScreenRect();
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
            WidgetUtils::DrawRect(
                game->mSpriteBatch, rect, SDL_Color{.r = 100, .g = 100, .b = 100, .a = 255});
        }

        SDL_Rect textRect = rect;
        // Apply slight offset if pressed
        if (button->IsPressed())
        {
            textRect.x += 1;
            textRect.y += 1;
        }

        game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
                                        game->mSpriteBatch,
                                        button->GetDisplayCaption(),
                                        textRect,
                                        textColor,
                                        13,
                                        TextAlignment::Center,
                                        true,
                                        button->GetMnemonicIndex());

        if (button->GetButtonStyle().showFocusRing && button->IsFocused() && !button->IsHovered())
        {
            const SDL_Rect focusRect{.x = rect.x + 2,
                                     .y = rect.y + 2,
                                     .w = SDL_max(0, rect.w - 4),
                                     .h = SDL_max(0, rect.h - 4)};
            WidgetUtils::DrawRect(
                game->mSpriteBatch, focusRect, SDL_Color{.r = 255, .g = 255, .b = 255, .a = 64});
        }
    }

    void JWindowAlphaTheme::DrawMdiWindow(Game* game, MdiWindow* window)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            window == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_Rect windowRect = window->GetScreenRect();
        const SDL_Rect titleRect = window->GetTitleBarRect();

        // 1. Draw Border and Background
        if (mOptions->mWindowBorder != nullptr)
        {
            mOptions->mWindowBorder->Draw(game->mSpriteBatch, windowRect, GetInnerRect(window));
        }
        else
        {
            WidgetUtils::FillRect(game->mSpriteBatch, windowRect, mOptions->mBackgroundColor);
            WidgetUtils::DrawRect(
                game->mSpriteBatch, windowRect, SDL_Color{.r = 100, .g = 100, .b = 100, .a = 255});
        }

        // 2. Draw Title
        int topFrameHeight = 0;
        if (mOptions->mWindowBorder != nullptr &&
            mOptions->mWindowBorder->mTopCenterHighlightFrame != nullptr)
        {
            if (SDL_Surface* surface =
                    mOptions->mWindowBorder->mTopCenterHighlightFrame->GetSurface())
            {
                topFrameHeight = surface->h;
            }
        }

        const int titleX = titleRect.x + (titleRect.w / 2);
        const int titleY = titleRect.y + topFrameHeight + mOptions->mWindowHeadingOffset;

        game->mFont->DrawStringWithFontAt(mOptions->mWindowHeadingFont,
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

        const SDL_Rect rect = textBox->GetScreenRect();
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

        SDL_Rect textRect{.x = rect.x + 6,
                          .y = rect.y + 3,
                          .w = SDL_max(0, rect.w - 12),
                          .h = SDL_max(0, rect.h - 6)};

        game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
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
            game->mFont->MeasureStringWithFont(mOptions->mGeneralFont,
                                               textBox->GetText(),
                                               style.fontPointSize,
                                               textWidth,
                                               textHeight);

            const bool visibleCaret = ((SDL_GetTicks() / 500U) % 2U) == 0U;
            if (visibleCaret)
            {
                SDL_Rect caretRect{.x = rect.x + 6 + textWidth,
                                   .y = rect.y + 4,
                                   .w = 1,
                                   .h = SDL_max(0, rect.h - 8)};
                WidgetUtils::FillRect(game->mSpriteBatch, caretRect, style.caretColor);
            }
        }
    }

    SDL_Rect JWindowAlphaTheme::GetInnerRect(const MdiWindow* window) const
    {
        // FIXME: hardcoded for now.
        SDL_Rect titleBarRect = window->GetScreenRect();
        return {.x = titleBarRect.x + 20,
                .y = titleBarRect.y + 30,
                .w = titleBarRect.w - 40,
                .h = titleBarRect.h - 50};
    }

    SDL_Rect JWindowAlphaTheme::GetCloseButtonRect(const MdiWindow* window) const
    {
        // FIXME: determine close button rect based on window rect and style.
        // We are currently reusing the metrics for classic theme.
        return window->GetCloseButtonRect();
    }
} // namespace nuvelocity
