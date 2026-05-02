#include "JWindowClassicTheme.h"
#include "ClassicSkinBorder.h"
#include <Colors.h>
#include <system/FontManager.h>
#include <system/Game.h>
#include <system/SpriteBatch.h>
#include <system/ui/Button.h>
#include "system/ui/JListBox.h"
#include "system/ui/JTabControl.h"
#include "system/ui/MdiWindow.h"
#include "system/ui/ScrollView.h"
#include "system/ui/TextBox.h"
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

        const SDL_Rect screenRect = button->GetScreenRect();
        const Insets margin = button->GetStyle().margin;
        SDL_Rect rect = {.x = screenRect.x + margin.left,
                         .y = screenRect.y + margin.top,
                         .w = screenRect.w,
                         .h = screenRect.h};
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
                                button->GetMnemonicIndex(),
                                buttonStyle.textColor);

        if (buttonStyle.showFocusRing && button->IsFocused() && !button->IsHovered())
        {
            const SDL_Rect focusRect{.x = rect.x + 2,
                                     .y = rect.y + 2,
                                     .w = SDL_max(0, rect.w - 4),
                                     .h = SDL_max(0, rect.h - 4)};
            WidgetUtils::DrawRect(
                game->mSpriteBatch, focusRect, SDL_Color{.r = 255, .g = 255, .b = 255, .a = 64});
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
                                true,
                                -1,
                                windowStyle.titleTextColor);

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
                                    true,
                                    -1,
                                    windowStyle.titleTextColor);
        }
    }

    void JWindowClassicTheme::DrawTextBox(Game* game, TextBox* textBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            textBox == nullptr)
        {
            return;
        }

        const SDL_Rect screenRect = textBox->GetScreenRect();
        const Insets margin = textBox->GetStyle().margin;
        SDL_Rect rect = {.x = screenRect.x + margin.left,
                         .y = screenRect.y + margin.top,
                         .w = screenRect.w,
                         .h = screenRect.h};
        const auto& style = textBox->GetTextBoxStyle();
        const auto& baseStyle = textBox->GetStyle();
        const std::string& text = textBox->GetText();
        const int scrollOffset = textBox->GetScrollOffset();

        const SDL_Color fillColor =
            textBox->IsFocused() ? style.focusedColor : style.unfocusedColor;

        WidgetUtils::FillRect(game->mSpriteBatch, rect, fillColor);
        WidgetUtils::DrawBevel(game->mSpriteBatch,
                               rect,
                               WidgetUtils::BevelColors{.light = baseStyle.borderLightColor,
                                                        .dark = baseStyle.borderDarkColor},
                               true,
                               baseStyle.borderThickness);

        const SDL_Rect textRect{.x = rect.x + 6,
                                .y = rect.y + 3,
                                .w = SDL_max(0, rect.w - 12),
                                .h = SDL_max(0, rect.h - 6)};

        // Clip all text-area drawing to textRect
        game->mSpriteBatch->SetClipRect(&textRect);

        // Draw selection highlight (positions offset by scroll)
        if (textBox->IsFocused() && textBox->HasSelection())
        {
            std::size_t selStart = 0;
            std::size_t selEnd = 0;
            textBox->GetSelection(selStart, selEnd);

            int xStart = 0;
            int xEnd = 0;
            int dummy = 0;
            game->mFont->MeasureString(
                text.substr(0, selStart), style.fontPointSize, xStart, dummy);
            game->mFont->MeasureString(text.substr(0, selEnd), style.fontPointSize, xEnd, dummy);

            const SDL_Rect selRect{.x = textRect.x + xStart - scrollOffset,
                                   .y = textRect.y,
                                   .w = SDL_max(0, xEnd - xStart),
                                   .h = textRect.h};
            WidgetUtils::FillRect(game->mSpriteBatch, selRect, style.selectionColor);
        }

        // Draw text shifted left by scrollOffset
        SDL_Rect shiftedTextRect = textRect;
        shiftedTextRect.x -= scrollOffset;
        shiftedTextRect.w += scrollOffset;
        game->mFont->DrawString(game->mSpriteBatch,
                                text,
                                shiftedTextRect,
                                style.textColor,
                                style.fontPointSize,
                                TextAlignment::Left,
                                true,
                                -1,
                                style.textColor);

        if (textBox->IsFocused())
        {
            const bool visibleCaret = textBox->IsCaretVisible();
            if (visibleCaret)
            {
                int caretPixelX = 0;
                int dummy = 0;
                game->mFont->MeasureString(text.substr(0, textBox->GetCaretPos()),
                                           style.fontPointSize,
                                           caretPixelX,
                                           dummy);

                if (textBox->IsOverwriteMode() && textBox->GetCaretPos() < text.size())
                {
                    // Block caret: cover the full width of the character under the caret
                    int nextX = 0;
                    game->mFont->MeasureString(text.substr(0, textBox->GetCaretPos() + 1),
                                               style.fontPointSize,
                                               nextX,
                                               dummy);

                    SDL_Rect caretRect{.x = textRect.x + caretPixelX - scrollOffset,
                                       .y = rect.y + 4,
                                       .w = SDL_max(2, nextX - caretPixelX),
                                       .h = SDL_max(0, rect.h - 8)};
                    // style.caretColor
                    WidgetUtils::FillRect(game->mSpriteBatch, caretRect, style.caretColor);
                }
                else
                {
                    SDL_Rect caretRect{.x = textRect.x + caretPixelX - scrollOffset,
                                       .y = rect.y + 4,
                                       .w = 1,
                                       .h = SDL_max(0, rect.h - 8)};
                    WidgetUtils::FillRect(game->mSpriteBatch, caretRect, style.caretColor);
                }
            }
        }

        game->mSpriteBatch->SetClipRect(nullptr);
    }

    int
    JWindowClassicTheme::MeasureTextWidth(Game* game, const std::string& text, int pointSize) const
    {
        if (game == nullptr || game->mFont == nullptr)
        {
            return 0;
        }
        int w = 0;
        int h = 0;
        game->mFont->MeasureString(text, pointSize, w, h);
        return w;
    }

    void JWindowClassicTheme::DrawListBox(Game* game, JListBox* listBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            listBox == nullptr)
        {
            return;
        }

        const SDL_Rect screenRect = listBox->GetScreenRect();
        const Insets margin = listBox->GetStyle().margin;
        SDL_Rect rect = {.x = screenRect.x + margin.left,
                         .y = screenRect.y + margin.top,
                         .w = screenRect.w,
                         .h = screenRect.h};
        const auto& style = listBox->GetStyle();
        SpriteBatch* batch = game->mSpriteBatch;

        // Draw background
        WidgetUtils::FillRect(batch, rect, Colors::Black);
        WidgetUtils::DrawBevel(
            batch, rect, {style.borderLightColor, style.borderDarkColor}, true, 1);

        // Draw Rows
        const auto& rows = listBox->GetRows();
        const auto& columns = listBox->GetColumns();
        int headerHeight = listBox->GetHeaderHeight();
        int rowHeight = listBox->GetRowHeight();
        int scrollOffset = listBox->GetScrollOffset();
        int selectedIndex = listBox->GetSelectedIndex();

        int maxVisibleRows = (rect.h - headerHeight) / rowHeight;
        for (int i = 0; i < maxVisibleRows; ++i)
        {
            int rowIndex = i + (scrollOffset / rowHeight);
            if (rowIndex >= static_cast<int>(rows.size()))
                break;

            int ry = rect.y + headerHeight + (i * rowHeight);

            if (rowIndex == selectedIndex)
            {
                WidgetUtils::FillRect(
                    batch, {rect.x + 1, ry, rect.w - 2, rowHeight}, {0, 0, 128, 255});
            }

            int curX = rect.x;
            const auto& row = rows[rowIndex];
            for (size_t c = 0; c < columns.size(); ++c)
            {
                if (c < row.size())
                {
                    game->mFont->DrawString(batch,
                                            row[c],
                                            {curX + 4, ry + 2, columns[c].width - 8, rowHeight - 4},
                                            Colors::White,
                                            8);
                }
                curX += columns[c].width;
            }
        }
    }

    void JWindowClassicTheme::DrawScrollView(Game* game, ScrollView* scrollView)
    {
        // Simple stub: draw background and scrollbar border
        if (game == nullptr || game->mSpriteBatch == nullptr || scrollView == nullptr)
        {
            return;
        }

        const SDL_Rect rect = scrollView->GetScreenRect();
        const auto& style = scrollView->GetStyle();
        SpriteBatch* batch = game->mSpriteBatch;

        WidgetUtils::DrawBevel(
            batch, rect, {style.borderLightColor, style.borderDarkColor}, true, 1);
    }

    void JWindowClassicTheme::DrawTabControl(Game* game, JTabControl* tabControl)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            tabControl == nullptr)
        {
            return;
        }

        const SDL_Rect screenRect = tabControl->GetScreenRect();
        const std::vector<std::string>& tabs = tabControl->GetTabs();
        const int selectedIndex = tabControl->GetSelectedIndex();
        const int padding = tabControl->GetTabPadding();
        const auto& style = tabControl->GetStyle();
        const int pointSize = GetTabPointSize();

        int curX = screenRect.x;
        for (int i = 0; i < static_cast<int>(tabs.size()); ++i)
        {
            int textWidth = MeasureTextWidth(game, tabs[i], pointSize);
            int tabWidth = textWidth + (padding * 2);
            SDL_Rect tabRect = {curX, screenRect.y, tabWidth, screenRect.h};

            bool selected = (i == selectedIndex);
            if (selected)
            {
                tabRect.y -= 2;
                tabRect.h += 2;
            }

            WidgetUtils::FillRect(game->mSpriteBatch, tabRect, style.backgroundColor);
            WidgetUtils::DrawBevel(game->mSpriteBatch,
                                   tabRect,
                                   {style.borderLightColor, style.borderDarkColor},
                                   false,
                                   1);

            if (selected)
            {
                // Remove bottom border for selected tab
                SDL_Rect bottomLine = {tabRect.x + 1, tabRect.y + tabRect.h - 1, tabRect.w - 2, 1};
                WidgetUtils::FillRect(game->mSpriteBatch, bottomLine, style.backgroundColor);
            }

            game->mFont->DrawString(game->mSpriteBatch,
                                    tabs[i],
                                    tabRect,
                                    Colors::Black,
                                    pointSize,
                                    TextAlignment::Center);

            curX += tabWidth;
        }

        // Draw a line under the tabs (except for the selected one)
        game->mSpriteBatch->DrawLine(screenRect.x,
                                     screenRect.y + screenRect.h - 1,
                                     screenRect.x + screenRect.w,
                                     screenRect.y + screenRect.h - 1,
                                     style.borderDarkColor);
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
