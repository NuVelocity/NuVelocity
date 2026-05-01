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

        const auto lines = BuildWrappedLines(game, mText, mRect.w);
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
            game->mFont->DrawStringWithFontAt(mFont,
                                              game->mSpriteBatch,
                                              mText,
                                              rect.x,
                                              rect.y,
                                              Colors::White,
                                              mPointSize,
                                              TextAlignment::Left,
                                              nullptr,
                                              -1,
                                              Colors::White);
            return;
        }

        const auto lines = BuildWrappedLines(game, mText, rect.w);
        const int lineHeight = MeasureLineHeight(game);
        SDL_Rect clipRect = rect;
        int drawY = rect.y;
        for (const auto& line : lines)
        {
            game->mFont->DrawStringWithFontAt(mFont,
                                              game->mSpriteBatch,
                                              line,
                                              rect.x,
                                              drawY,
                                              Colors::White,
                                              mPointSize,
                                              TextAlignment::Left,
                                              &clipRect,
                                              -1,
                                              Colors::White);
            drawY += lineHeight;
        }
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
} // namespace nuvelocity
