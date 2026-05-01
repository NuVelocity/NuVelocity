#include "JWindowAlphaTheme.h"
#include "AlphaSkinBorder.h"
#include "Colors.h"
#include "StandAloneFrame.h"
#include "system/FontManager.h"
#include "system/Game.h"
#include "system/SpriteBatch.h"
#include "system/ui/Button.h"
#include "system/ui/JListBox.h"
#include "system/ui/JTabControl.h"
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

        const SDL_Rect screenRect = button->GetScreenRect();
        const Insets margin = button->GetStyle().margin;
        SDL_Rect rect = {.x = screenRect.x + margin.left,
                         .y = screenRect.y + margin.top,
                         .w = screenRect.w,
                         .h = screenRect.h};
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
            border->DrawBackground(game->mSpriteBatch, rect);
            border->DrawBorder(game->mSpriteBatch, rect);
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
                                        button->GetMnemonicIndex(),
                                        // FIXME: we don't respect text color for now.
                                        mOptions->mShortcutKeyHighlightColor);

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

        if (window->IsFullScreen())
        {
            SDL_Rect screenRect = {0, 0, game->mWindowWidth, game->mWindowHeight};
            WidgetUtils::FillRect(game->mSpriteBatch, screenRect, Colors::Black);
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
                                          TextAlignment::Center,
                                          nullptr,
                                          -1,
                                          window->GetWindowStyle().titleTextColor);
    }

    void JWindowAlphaTheme::DrawTextBox(Game* game, TextBox* textBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            textBox == nullptr || mOptions == nullptr)
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
        const std::string& text = textBox->GetText();
        const int scrollOffset = textBox->GetScrollOffset();

        SDL_Rect borderRect = {.x = rect.x + 1, .y = rect.y + 1, .w = rect.w - 2, .h = rect.h - 2};
        mOptions->mButtonBorder->DrawBorder(game->mSpriteBatch, borderRect);
        WidgetUtils::DrawRect(game->mSpriteBatch, rect, Colors::Black);

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
            game->mFont->MeasureStringWithFont(mOptions->mGeneralFont,
                                               text.substr(0, selStart),
                                               13, // style.fontPointSize,
                                               xStart,
                                               dummy);
            game->mFont->MeasureStringWithFont(mOptions->mGeneralFont,
                                               text.substr(0, selEnd),
                                               13, // style.fontPointSize,
                                               xEnd,
                                               dummy);

            const SDL_Rect selRect{.x = textRect.x + xStart - scrollOffset,
                                   .y = textRect.y,
                                   .w = SDL_max(0, xEnd - xStart),
                                   .h = textRect.h};
            WidgetUtils::FillRect(game->mSpriteBatch, selRect, mOptions->mSelectColor);
        }

        // Draw text shifted left by scrollOffset
        SDL_Rect shiftedTextRect = textRect;
        shiftedTextRect.x -= scrollOffset;
        shiftedTextRect.w += scrollOffset;
        game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
                                        game->mSpriteBatch,
                                        text,
                                        shiftedTextRect,
                                        // FIXME: we don't respect text color for now.
                                        mOptions->mShortcutKeyHighlightColor, // style.textColor,
                                        13, // style.fontPointSize,
                                        TextAlignment::Left,
                                        true,
                                        -1,
                                        // FIXME: we don't respect text color for now.
                                        mOptions->mShortcutKeyHighlightColor // style.textColor
        );

        if (textBox->IsFocused())
        {
            const bool visibleCaret = textBox->IsCaretVisible();
            if (visibleCaret)
            {
                int caretPixelX = 0;
                int dummy = 0;
                game->mFont->MeasureStringWithFont(mOptions->mGeneralFont,
                                                   text.substr(0, textBox->GetCaretPos()),
                                                   13, // style.fontPointSize,
                                                   caretPixelX,
                                                   dummy);

                if (textBox->IsOverwriteMode() && textBox->GetCaretPos() < text.size())
                {
                    // Block caret: cover the full width of the character under the caret
                    int nextX = 0;
                    game->mFont->MeasureStringWithFont(mOptions->mGeneralFont,
                                                       text.substr(0, textBox->GetCaretPos() + 1),
                                                       13, // style.fontPointSize,
                                                       nextX,
                                                       dummy);

                    SDL_Rect caretRect{.x = textRect.x + caretPixelX - scrollOffset,
                                       .y = rect.y + 4,
                                       .w = SDL_max(2, nextX - caretPixelX),
                                       .h = SDL_max(0, rect.h - 8)};
                    WidgetUtils::FillRect(game->mSpriteBatch,
                                          caretRect,
                                          // FIXME: we don't respect caret color for now.
                                          mOptions->mShortcutKeyHighlightColor
                                          // style.caretColor
                    );
                }
                else
                {
                    SDL_Rect caretRect{.x = textRect.x + caretPixelX - scrollOffset,
                                       .y = rect.y + 4,
                                       .w = 1,
                                       .h = SDL_max(0, rect.h - 8)};
                    WidgetUtils::FillRect(game->mSpriteBatch,
                                          caretRect,
                                          // FIXME: we don't respect caret color for now.
                                          mOptions->mShortcutKeyHighlightColor
                                          // style.caretColor
                    );
                }
            }
        }

        game->mSpriteBatch->SetClipRect(nullptr);
    }

    int
    JWindowAlphaTheme::MeasureTextWidth(Game* game, const std::string& text, int pointSize) const
    {
        if (game == nullptr || game->mFont == nullptr || mOptions == nullptr)
        {
            return 0;
        }
        int w = 0;
        int h = 0;
        game->mFont->MeasureStringWithFont(mOptions->mGeneralFont, text, pointSize, w, h);
        return w;
    }

    void JWindowAlphaTheme::DrawListBox(Game* game, JListBox* listBox)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            listBox == nullptr || mOptions == nullptr)
        {
            return;
        }

        const SDL_Rect screenRect = listBox->GetScreenRect();
        const Insets margin = listBox->GetStyle().margin;
        SDL_Rect rect = {.x = screenRect.x + margin.left,
                         .y = screenRect.y + margin.top,
                         .w = screenRect.w,
                         .h = screenRect.h};
        SpriteBatch* batch = game->mSpriteBatch;

        // Use colors from skin data
        const SDL_Color bgColor = mOptions->mBackgroundColor;
        const SDL_Color selectColor = mOptions->mSelectColor;
        const SDL_Color textColor = Colors::White;
        const SDL_Color separatorColor = {176, 148, 56, 255};

        // Draw Header
        int fw = 0, fh = 0;
        game->mFont->MeasureStringWithFont(mOptions->mGeneralFont, "Wy", 13, fw, fh);
        listBox->SetHeaderHeight(fh + 2);
        listBox->SetRowHeight(fh + 2);

        int headerHeight = listBox->GetHeaderHeight();
        SDL_Rect headerRect = {rect.x, rect.y, rect.w, headerHeight};

        SDL_Rect blackArea = {
            rect.x + 1, rect.y + headerHeight + 1, rect.w - 2, rect.h - headerHeight - 2};
        WidgetUtils::DrawRect(batch, blackArea, Colors::Black);

        SDL_Rect buttonBorderArea = blackArea;
        buttonBorderArea.x += 1;
        buttonBorderArea.y += 1;
        buttonBorderArea.w -= 2;
        buttonBorderArea.h -= 2;
        mOptions->mButtonBorder->DrawBorder(game->mSpriteBatch, buttonBorderArea);

        // Content area
        int marginOffset = mOptions->mButtonBorder->mTextureMargin + 2;

        SDL_Rect contentClip = {.x = rect.x,
                                .y = rect.y + headerHeight + 1 + marginOffset,
                                .w = rect.w - 16,
                                .h = rect.h - headerHeight - 2 - marginOffset};
        const auto& columns = listBox->GetColumns();
        int curX = rect.x;
        for (const auto& col : columns)
        {
            game->mFont->DrawStringWithFontAt(
                mOptions->mGeneralFont, batch, col.title, curX + 4, rect.y + 1, Colors::White, 13);
            curX += col.width;
            batch->DrawLine(curX, rect.y, curX, rect.y + headerHeight, separatorColor);
        }

        // Yellow separator line
        batch->DrawLine(
            rect.x, rect.y + headerHeight, rect.x + rect.w, rect.y + headerHeight, separatorColor);

        // Draw Rows
        const auto& rows = listBox->GetRows();
        int rowHeight = listBox->GetRowHeight();
        int scrollOffset = listBox->GetScrollOffset();
        int selectedIndex = listBox->GetSelectedIndex();

        int maxVisibleRows = (rect.h - headerHeight) / rowHeight;

        batch->SetClipRect(&contentClip);

        int leftMargin = 5;
        int rightMargin = 4;

        for (int i = 0; i < maxVisibleRows + 1; ++i)
        {
            int rowIndex = i + (scrollOffset / rowHeight);
            if (rowIndex >= static_cast<int>(rows.size()))
            {
                break;
            }

            int ry =
                rect.y + headerHeight + marginOffset + (i * rowHeight) - (scrollOffset % rowHeight);

            if (rowIndex == selectedIndex)
            {
                WidgetUtils::FillRect(
                    batch,
                    {rect.x + leftMargin, ry, rect.w - 16 - rightMargin - leftMargin, rowHeight},
                    selectColor);
            }

            curX = rect.x + leftMargin;
            const auto& row = rows[rowIndex];
            for (size_t c = 0; c < columns.size(); ++c)
            {
                if (c < row.size())
                {
                    // Clip each column
                    SDL_Rect colClip = {curX, ry, columns[c].width, rowHeight};
                    // Intersection with row area clip
                    colClip.x = SDL_max(colClip.x, contentClip.x);
                    colClip.y = SDL_max(colClip.y, contentClip.y);
                    int right = SDL_min(colClip.x + colClip.w, contentClip.x + contentClip.w);
                    int bottom = SDL_min(colClip.y + colClip.h, contentClip.y + contentClip.h);
                    colClip.w = SDL_max(0, right - colClip.x);
                    colClip.h = SDL_max(0, bottom - colClip.y);

                    batch->SetClipRect(&colClip);
                    game->mFont->DrawStringWithFontAt(mOptions->mGeneralFont,
                                                      batch,
                                                      row[c],
                                                      curX + 4,
                                                      ry + 1,
                                                      textColor,
                                                      13,
                                                      TextAlignment::Left,
                                                      nullptr,
                                                      -1,
                                                      textColor);
                    batch->SetClipRect(&contentClip);
                }
                curX += columns[c].width;
            }
        }
        batch->SetClipRect(nullptr);

        // Draw Scrollbar
        int sbX = rect.x + rect.w - 16;
        SDL_Rect upBtn = {sbX, rect.y + headerHeight + 1, 15, 15};
        SDL_Rect downBtn = {sbX, rect.y + rect.h - 16, 15, 15};

        // Scrollbar Track Container
        SDL_Rect trackRect = {sbX, upBtn.y + 15, 15, downBtn.y - (upBtn.y + 15)};
        WidgetUtils::FillRect(batch, trackRect, {0, 0, 0, 150}); // Black semi-transparent
        mOptions->mButtonBorder->DrawBorder(batch, trackRect, true);

        // Draw Up/Down buttons
        if (mOptions->mWindowBorder != nullptr)
        {
            mOptions->mWindowBorder->DrawTiledBackground(batch, upBtn);
            mOptions->mWindowBorder->DrawTiledBackground(batch, downBtn);
        }
        mOptions->mButtonBorder->DrawBorder(batch, upBtn, listBox->IsUpPressed());
        mOptions->mButtonBorder->DrawBorder(batch, downBtn, listBox->IsDownPressed());

        // Font-based arrows
        std::string upStr(1, (char)28);
        std::string downStr(1, (char)29);

        SDL_Rect upIconRect = {.x = upBtn.x, .y = upBtn.y + 2, .w = upBtn.w, .h = upBtn.h};
        if (listBox->IsUpPressed())
        {
            upIconRect.x += 1;
            upIconRect.y += 1;
        }

        SDL_Rect downIconRect = {
            .x = downBtn.x, .y = downBtn.y + 2, .w = downBtn.w, .h = downBtn.h};
        if (listBox->IsDownPressed())
        {
            downIconRect.x += 1;
            downIconRect.y += 1;
        }

        game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
                                        batch,
                                        upStr,
                                        upIconRect,
                                        Colors::White,
                                        13,
                                        TextAlignment::Center);
        game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
                                        batch,
                                        downStr,
                                        downIconRect,
                                        Colors::White,
                                        13,
                                        TextAlignment::Center);

        // Thumb (dynamic-ish sizing)
        int totalHeight = (int)rows.size() * rowHeight;
        int listAreaHeight = rect.h - headerHeight;
        int trackHeight = downBtn.y - (upBtn.y + 15);

        if (totalHeight > listAreaHeight)
        {
            float ratio = (float)listAreaHeight / (float)totalHeight;
            int thumbHeight = (int)(trackHeight * ratio);
            thumbHeight = SDL_max(15, thumbHeight);

            int maxScroll = totalHeight - listAreaHeight;
            int thumbY = upBtn.y + 15;
            if (maxScroll > 0)
            {
                thumbY += (scrollOffset * (trackHeight - thumbHeight)) / maxScroll;
            }

            SDL_Rect thumbRect = {sbX, thumbY, 15, thumbHeight};

            if (mOptions->mWindowBorder != nullptr)
            {
                mOptions->mWindowBorder->DrawTiledBackground(batch, thumbRect);
            }
            mOptions->mButtonBorder->DrawBorder(batch, thumbRect, false);
        }
    }

    void JWindowAlphaTheme::DrawTabControl(Game* game, JTabControl* tabControl)
    {
        if (game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr ||
            tabControl == nullptr || mOptions == nullptr || mOptions->mButtonBorder == nullptr)
        {
            return;
        }

        const SDL_Rect screenRect = tabControl->GetScreenRect();
        const std::vector<std::string>& tabs = tabControl->GetTabs();
        const int selectedIndex = tabControl->GetSelectedIndex();
        const int padding = tabControl->GetTabPadding();
        const int pointSize = GetTabPointSize();

        ClassicSkinBorder* b = mOptions->mButtonBorder;
        ClassicSkinBorder* bPressed = mOptions->mPressedButtonBorder != nullptr
                                          ? mOptions->mPressedButtonBorder
                                          : mOptions->mButtonBorder;

        WidgetUtils::BorderColors borderColors = {.topLeftOuter = b->mBottomInnerColor,
                                                  .topLeftInner = b->mBottomOuterColor,
                                                  .bottomRightInner = b->mTopOuterColor,
                                                  .bottomRightOuter = b->mTopInnerColor};

        WidgetUtils::BorderColors pressedBorderColors = {
            .topLeftOuter = bPressed->mBottomInnerColor,
            .topLeftInner = bPressed->mBottomOuterColor,
            .bottomRightInner = bPressed->mTopOuterColor,
            .bottomRightOuter = bPressed->mTopInnerColor};

        // Continuous bottom strip under the tab row.
        const int headerHeight = tabControl->GetTabHeaderHeight();

        // Compute widths first so the tabs fill the control width.
        std::vector<int> tabWidths;
        tabWidths.reserve(tabs.size());
        int totalTabWidth = 0;
        for (size_t i = 0; i < tabs.size(); ++i)
        {
            int textWidth = MeasureTextWidth(game, tabs[i], pointSize);
            int tabWidth = textWidth + (padding * 2);
            tabWidths.push_back(tabWidth);
            totalTabWidth += tabWidth;
        }

        int curX = screenRect.x;
        int activeLeft = screenRect.x;
        int activeRight = screenRect.x;
        for (size_t i = 0; i < tabs.size(); ++i)
        {
            int tabWidth = tabWidths[i];
            bool selected = (static_cast<int>(i) == selectedIndex);

            // Inactive tabs sit 1px lower, like classic Win9x tabs.
            const int tabY = selected ? screenRect.y : (screenRect.y + 1);
            const int tabH = selected ? headerHeight : SDL_max(0, headerHeight - 1);
            SDL_Rect tabRect = {curX, tabY, tabWidth, tabH};

            int sides = WidgetUtils::Side_Top | WidgetUtils::Side_Left | WidgetUtils::Side_Right;

            const WidgetUtils::BorderColors& useColors =
                selected ? pressedBorderColors : borderColors;
            WidgetUtils::DrawBevel(
                game->mSpriteBatch, tabRect, useColors, b->mTextureMargin, sides);

            SDL_Rect textRect = {
                .x = tabRect.x, .y = tabRect.y + 6, .w = tabRect.w, .h = tabRect.h};
            if (selected)
            {
                textRect.y -= 1;
            }

            game->mFont->DrawStringWithFont(mOptions->mGeneralFont,
                                            game->mSpriteBatch,
                                            tabs[i],
                                            textRect,
                                            Colors::White,
                                            13,
                                            TextAlignment::Center);

            curX += tabWidth;

            if (selected)
            {
                activeLeft = tabRect.x;
                activeRight = tabRect.x + tabRect.w;
            }
        }

        // Content bevel starts at the tab span width so it aligns with the headers.
        const SDL_Rect tRect = tabControl->GetScreenRect();
        Widget* parent = tabControl->GetParent();
        SDL_Rect containerRect =
            parent != nullptr
                ? parent->GetScreenRect()
                : SDL_Rect{.x = 0, .y = 0, .w = game->mWindowWidth, .h = game->mWindowHeight};

        const int contentX = screenRect.x;
        const int contentY = screenRect.y + headerHeight;
        const int contentW = SDL_max(0, screenRect.w);
        const int contentH = SDL_max(0, screenRect.h - headerHeight);

        SDL_Rect contentRect{.x = contentX, .y = contentY, .w = contentW, .h = contentH};
        if (contentRect.w > 0 && contentRect.h > 0)
        {
            WidgetUtils::DrawBevel(game->mSpriteBatch,
                                   contentRect,
                                   pressedBorderColors,
                                   b->mTextureMargin,
                                   WidgetUtils::Side_Left | WidgetUtils::Side_Right |
                                       WidgetUtils::Side_Bottom);

            // Draw the content top border in two segments so it does not run
            // under the active tab.
            const int thickness = b->mTextureMargin;
            for (int i = 0; i < thickness; ++i)
            {
                const SDL_Color topColor = (i < (thickness / 2)) ? pressedBorderColors.topLeftOuter
                                                                 : pressedBorderColors.topLeftInner;

                const int y = contentRect.y + i;
                const int leftW = SDL_max(0, SDL_min(activeLeft - contentRect.x, contentRect.w));
                if (leftW > 0)
                {
                    SDL_Rect leftLine{.x = contentRect.x, .y = y, .w = leftW, .h = 1};
                    WidgetUtils::FillRect(game->mSpriteBatch, leftLine, topColor);
                }

                const int rightX = SDL_max(activeRight, contentRect.x);
                const int rightW = SDL_max(0, (contentRect.x + contentRect.w) - rightX);
                if (rightW > 0)
                {
                    SDL_Rect rightLine{.x = rightX, .y = y, .w = rightW, .h = 1};
                    WidgetUtils::FillRect(game->mSpriteBatch, rightLine, topColor);
                }
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
