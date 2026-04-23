#include "JWindowClassicTheme.h"
#include "ClassicSkinBorder.h"
#include <system/FontManager.h>
#include <system/Game.h>
#include <system/SpriteBatch.h>
#include <system/ui/Button.h>
#include <system/ui/MdiWindow.h>
#include <system/ui/TextBox.h>
#include <system/ui/WidgetUtils.h>

namespace nuvelocity
{
    void JWindowClassicTheme::DrawButton(Game* game, Button* button)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            button == nullptr)
        {
            return;
        }

        const SDL_Rect rect = button->GetScreenRect();
        const auto& style = button->GetStyle();
        const auto& buttonStyle = button->GetButtonStyle();

        // JWindowClassicTheme uses the standard hardcoded bevel for buttons
        SDL_Color color = style.backgroundColor;
        if (!button->IsEnabled())
        {
            color = style.disabledColor;
        }
        else if (button->IsPressed())
        {
            color = buttonStyle.pressedColor;
        }
        else if (button->IsHovered())
        {
            color = buttonStyle.hoverColor;
        }

        WidgetUtils::FillRect(game->mSpriteBatch, rect, color);
        WidgetUtils::DrawBevel(game->mSpriteBatch,
                               rect,
                               WidgetUtils::BevelColors{.light = style.borderLightColor,
                                                        .dark = style.borderDarkColor},
                               button->IsPressed(),
                               style.borderThickness);

        // 3. Draw Text
        SDL_Rect textRect{.x = rect.x + 4,
                           .y = rect.y + 2,
                           .w = SDL_max(0, rect.w - 8),
                           .h = SDL_max(0, rect.h - 4)};

        if (button->IsPressed())
        {
            textRect.x += 1;
            textRect.y += 1;
        }

        game->mFont->DrawString(game->mSpriteBatch,
                                button->GetDisplayCaption(),
                                textRect,
                                buttonStyle.textColor,
                                buttonStyle.fontPointSize,
                                TextAlignment::Center,
                                true,
                                button->GetMnemonicIndex());

        if (buttonStyle.showFocusRing && button->IsFocused() && !button->IsHovered())
        {
            const SDL_Rect focusRect{.x = rect.x + 2,
                                      .y = rect.y + 2,
                                      .w = SDL_max(0, rect.w - 4),
                                      .h = SDL_max(0, rect.h - 4)};
            WidgetUtils::DrawRect(game->mSpriteBatch, focusRect, SDL_Color{255, 255, 255, 64});
        }
    }

    void JWindowClassicTheme::DrawMdiWindow(Game* game, MdiWindow* window)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            window == nullptr)
        {
            return;
        }

        const SDL_Rect windowRect = window->GetScreenRect();
        const SDL_Rect titleRect = window->GetTitleBarRect();
        const SDL_Rect clientRect = window->GetClientRect();

        const auto& style = window->GetStyle();
        const auto& windowStyle = window->GetWindowStyle();

        // JWindowClassicTheme uses the standard hardcoded bevel for windows
        WidgetUtils::FillRect(game->mSpriteBatch, windowRect, style.backgroundColor);
        WidgetUtils::DrawBevel(game->mSpriteBatch,
                               windowRect,
                               WidgetUtils::BevelColors{.light = style.borderLightColor,
                                                        .dark = style.borderDarkColor},
                               false,
                               windowStyle.borderSize);

        // 2. Draw Title Bar
        const SDL_Color titleColor =
            window->IsActive() ? windowStyle.titleBarColor : windowStyle.titleBarInactiveColor;
        WidgetUtils::FillRect(game->mSpriteBatch, titleRect, titleColor);

        SDL_Rect titleTextRect{.x = titleRect.x + 6,
                                .y = titleRect.y,
                                .w = SDL_max(0, titleRect.w - 24),
                                .h = titleRect.h};

        game->mFont->DrawString(game->mSpriteBatch,
                                window->GetTitle(),
                                titleTextRect,
                                windowStyle.titleTextColor,
                                windowStyle.titleFontPointSize,
                                TextAlignment::Left,
                                true);

        // 3. Draw Client Background
        if (windowStyle.tileBackground && window->GetBackgroundTile() != nullptr)
        {
            WidgetUtils::DrawTiledFrame(
                game->mSpriteBatch, window->GetBackgroundTile(), clientRect);
        }
        else
        {
            WidgetUtils::FillRect(game->mSpriteBatch, clientRect, windowStyle.clientColor);
        }

        // 4. Draw Close Button
        if (window->IsClosable())
        {
            SDL_Rect closeRect = window->GetCloseButtonRect();
            WidgetUtils::FillRect(game->mSpriteBatch, closeRect, windowStyle.closeButtonColor);
            WidgetUtils::DrawBevel(game->mSpriteBatch,
                                   closeRect,
                                   WidgetUtils::BevelColors{.light = style.borderLightColor,
                                                            .dark = style.borderDarkColor},
                                   false,
                                   1);
            game->mFont->DrawString(game->mSpriteBatch,
                                    "X",
                                    closeRect,
                                    windowStyle.titleTextColor,
                                    windowStyle.titleFontPointSize,
                                    TextAlignment::Center,
                                    true);
        }
    }

    void JWindowClassicTheme::DrawTextBox(Game* game, TextBox* textBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            textBox == nullptr)
        {
            return;
        }

        const SDL_Rect rect = textBox->GetScreenRect();
        const auto& style = textBox->GetTextBoxStyle();
        const auto& baseStyle = textBox->GetStyle();

        const SDL_Color fillColor =
            textBox->IsFocused() ? style.focusedColor : style.unfocusedColor;

        WidgetUtils::FillRect(game->mSpriteBatch, rect, fillColor);
        WidgetUtils::DrawBevel(game->mSpriteBatch,
                               rect,
                               WidgetUtils::BevelColors{.light = baseStyle.borderLightColor,
                                                        .dark = baseStyle.borderDarkColor},
                               true,
                               baseStyle.borderThickness);

        SDL_Rect textRect{.x = rect.x + 6,
                           .y = rect.y + 3,
                           .w = SDL_max(0, rect.w - 12),
                           .h = SDL_max(0, rect.h - 6)};
        game->mFont->DrawString(game->mSpriteBatch,
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
            game->mFont->MeasureString(
                textBox->GetText(), style.fontPointSize, textWidth, textHeight);

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

    SDL_Rect JWindowClassicTheme::GetInnerRect(const MdiWindow* window) const
    {
        return window->GetTitleBarRect();
    }

    SDL_Rect JWindowClassicTheme::GetCloseButtonRect(const MdiWindow* window) const
    {
        return window->GetCloseButtonRect();
    }
} // namespace nuvelocity
