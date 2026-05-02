#include "Label.h"

#include "Colors.h"
#include "Game.h"
#include "system/FontManager.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <sstream>
#include <utility>

namespace nuvelocity
{
    Label::Label(std::string text, std::string font)
            : mText(std::move(text))
            , mFont(std::move(font))
    {
    }

    void Label::SetText(const std::string& text)
    {
        mText = text;
        InvalidateLayout();
    }
    const std::string& Label::GetText() const
    {
        return mText;
    }
    void Label::SetFont(const std::string& font)
    {
        mFont = font;
        InvalidateLayout();
    }
    const std::string& Label::GetFont() const
    {
        return mFont;
    }
    void Label::SetWrap(bool wrap)
    {
        mWrap = wrap;
        InvalidateLayout();
    }
    bool Label::IsWrapEnabled() const
    {
        return mWrap;
    }
    void Label::SetPointSize(int pointSize)
    {
        mPointSize = pointSize;
        InvalidateLayout();
    }
    int Label::GetPointSize() const
    {
        return mPointSize;
    }
    void Label::SetAlignment(TextAlignment alignment)
    {
        mAlignment = alignment;
    }
    TextAlignment Label::GetAlignment() const
    {
        return mAlignment;
    }
    void Label::SetColor(const SDL_Color& color)
    {
        mColor = color;
    }
    const SDL_Color& Label::GetColor() const
    {
        return mColor;
    }
    void Label::SetVerticalCenter(bool center)
    {
        mVerticalCenter = center;
    }
    bool Label::IsVerticalCenter() const
    {
        return mVerticalCenter;
    }

    void Label::Update(Game* game)
    {
        Widget::Update(game);

        if (!mWrap || mRect.w <= 0 || mRect.h != -1 || game == nullptr || game->mFont == nullptr)
        {
            return;
        }

        if (mText.empty())
        {
            mRect.h = 0;
            return;
        }

        const auto lines = GetWrappedLines(game, mRect.w);
        const int lineHeight = MeasureLineHeight(game);
        mRect.h = SDL_max(0, static_cast<int>(lines.size()) * lineHeight);
    }

    void Label::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mFont == nullptr || game->mSpriteBatch == nullptr)
        {
            return;
        }

        SDL_Rect rect = GetScreenRect();

        if (!mWrap || rect.w <= 0)
        {
            int drawX = rect.x;
            if (mAlignment == TextAlignment::Center)
            {
                drawX = rect.x + rect.w / 2;
            }
            else if (mAlignment == TextAlignment::Right)
            {
                drawX = rect.x + rect.w;
            }

            int drawY = rect.y;
            if (mVerticalCenter)
            {
                int width = 0;
                int height = 0;
                if (game->mFont->MeasureStringWithFont(mFont, mText, mPointSize, width, height))
                {
                    drawY = rect.y + (rect.h - height) / 2;
                }
            }

            game->mFont->DrawStringWithFontAt(mFont,
                                              game->mSpriteBatch,
                                              mText,
                                              drawX,
                                              drawY,
                                              mColor,
                                              mPointSize,
                                              mAlignment,
                                              nullptr,
                                              -1,
                                              mColor);
            return;
        }

        const auto lines = GetWrappedLines(game, rect.w);
        const int lineHeight = MeasureLineHeight(game);
        SDL_Rect clipRect = rect;
        int drawY = rect.y;

        if (mVerticalCenter)
        {
            int totalHeight = static_cast<int>(lines.size()) * lineHeight;
            drawY = rect.y + (rect.h - totalHeight) / 2;
        }

        for (const auto& line : lines)
        {
            int drawX = rect.x;
            if (mAlignment == TextAlignment::Center)
            {
                drawX = rect.x + rect.w / 2;
            }
            else if (mAlignment == TextAlignment::Right)
            {
                drawX = rect.x + rect.w;
            }

            game->mFont->DrawStringWithFontAt(mFont,
                                              game->mSpriteBatch,
                                              line,
                                              drawX,
                                              drawY,
                                              mColor,
                                              mPointSize,
                                              mAlignment,
                                              &clipRect,
                                              -1,
                                              mColor);
            drawY += lineHeight;
        }
    }

    std::vector<std::string> Label::GetWrappedLines(Game* game, int maxWidth) const
    {
        if (maxWidth == mLastWrapWidth && mText == mLastWrapText && !mCachedLines.empty())
        {
            return mCachedLines;
        }

        mCachedLines = BuildWrappedLines(game, mText, maxWidth);
        mLastWrapWidth = maxWidth;
        mLastWrapText = mText;
        return mCachedLines;
    }

    int Label::GetRequiredHeight(Game* game, int maxWidth) const
    {
        const auto lines = GetWrappedLines(game, maxWidth);
        return static_cast<int>(lines.size()) * MeasureLineHeight(game);
    }

    std::vector<std::string>
    Label::BuildWrappedLines(Game* game, const std::string& text, int maxWidth) const
    {
        std::vector<std::string> lines;
        if (game == nullptr || game->mFont == nullptr || maxWidth <= 0)
        {
            lines.push_back(text);
            return lines;
        }

        const auto measureWidth = [&](const std::string& value) -> int
        {
            int width = 0;
            int height = 0;
            if (game->mFont->MeasureStringWithFont(mFont, value, mPointSize, width, height))
            {
                return width;
            }
            if (game->mFont->MeasureString(value, mPointSize, width, height))
            {
                return width;
            }
            return 0;
        };

        std::stringstream paragraphStream(text);
        std::string paragraph;
        while (std::getline(paragraphStream, paragraph))
        {
            std::istringstream wordStream(paragraph);
            std::string word;
            std::string currentLine;
            while (wordStream >> word)
            {
                std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
                if (measureWidth(candidate) <= maxWidth || currentLine.empty())
                {
                    currentLine = candidate;
                }
                else
                {
                    lines.push_back(currentLine);
                    currentLine.clear();

                    if (measureWidth(word) <= maxWidth)
                    {
                        currentLine = word;
                        continue;
                    }

                    std::string fragment;
                    for (char ch : word)
                    {
                        std::string fragmentCandidate = fragment + ch;
                        if (measureWidth(fragmentCandidate) > maxWidth && !fragment.empty())
                        {
                            lines.push_back(fragment);
                            fragment.clear();
                        }
                        fragment.push_back(ch);
                    }
                    currentLine = fragment;
                }
            }

            if (!currentLine.empty() || paragraph.empty())
            {
                lines.push_back(currentLine);
            }
        }

        if (lines.empty())
        {
            lines.push_back(std::string());
        }

        return lines;
    }

    int Label::MeasureLineHeight(Game* game) const
    {
        if (game == nullptr || game->mFont == nullptr)
        {
            return mPointSize;
        }

        int width = 0;
        int height = 0;
        if (game->mFont->MeasureStringWithFont(mFont, "Ag", mPointSize, width, height))
        {
            return height;
        }
        if (game->mFont->MeasureString("Ag", mPointSize, width, height))
        {
            return height;
        }

        return mPointSize;
    }

    void Label::InvalidateLayout()
    {
        mLastWrapWidth = -1;
        mCachedLines.clear();
    }
} // namespace nuvelocity
