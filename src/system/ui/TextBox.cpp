#include "TextBox.h"

#include "WidgetUtils.h"

#include <system/FontManager.h>
#include <system/Game.h>
#include <system/InputManager.h>

namespace nuvelocity
{
    TextBox::TextBox()
            : mMaxLength(256)
            , mReadOnly(false)
            , mFocused(false)
    {
        Widget::SetStyle(mTextBoxStyle.baseStyle);
    }

    void TextBox::Update(InputManager& input, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || !mEnabled)
        {
            return;
        }

        const SDL_FPoint mouse = input.GetMousePosition();
        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            mFocused = ContainsPoint(mouse, parentOffset);
        }

        if (!mFocused || mReadOnly)
        {
            return;
        }

        bool changed = false;
        const std::string& textInput = input.GetTextInput();
        for (char ch : textInput)
        {
            if (ch < 32 || ch > 126)
            {
                continue;
            }

            if (mText.size() >= mMaxLength)
            {
                break;
            }

            mText.push_back(ch);
            changed = true;
        }

        if (input.IsKeyPressed(SDL_SCANCODE_BACKSPACE) && !mText.empty())
        {
            mText.pop_back();
            changed = true;
        }

        if (changed && mOnTextChanged)
        {
            mOnTextChanged(mText);
        }

        if (input.IsKeyPressed(SDL_SCANCODE_RETURN) && mOnSubmit)
        {
            mOnSubmit(mText);
        }
    }

    void TextBox::Draw(Game* game, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_FRect rect = GetScreenRect(parentOffset);
        const SDL_Color fillColor =
            mFocused ? mTextBoxStyle.focusedColor : mTextBoxStyle.unfocusedColor;

        FillRect(game->mSpriteBatch, rect, fillColor);
        DrawBevel(game->mSpriteBatch,
                  rect,
                  BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                  true,
                  mStyle.borderThickness);

        SDL_FRect textRect{.x = rect.x + 6.0F,
                           .y = rect.y + 3.0F,
                           .w = SDL_max(0.0F, rect.w - 12.0F),
                           .h = SDL_max(0.0F, rect.h - 6.0F)};
        game->mFont->DrawString(game->mSpriteBatch,
                                mText,
                                textRect,
                                mTextBoxStyle.textColor,
                                mTextBoxStyle.fontPointSize,
                                TextAlignment::Left,
                                true);

        if (mFocused)
        {
            int textWidth = 0;
            int textHeight = 0;
            game->mFont->MeasureString(mText, mTextBoxStyle.fontPointSize, textWidth, textHeight);

            const bool visibleCaret = ((SDL_GetTicks() / 500U) % 2U) == 0U;
            if (visibleCaret)
            {
                SDL_FRect caretRect{.x = rect.x + 6.0F + static_cast<float>(textWidth),
                                    .y = rect.y + 4.0F,
                                    .w = 1.0F,
                                    .h = SDL_max(0.0F, rect.h - 8.0F)};
                FillRect(game->mSpriteBatch, caretRect, mTextBoxStyle.caretColor);
            }
        }
    }

    void TextBox::SetText(const std::string& text)
    {
        mText = text.substr(0, mMaxLength);
    }

    const std::string& TextBox::GetText() const
    {
        return mText;
    }

    void TextBox::SetReadOnly(bool readOnly)
    {
        mReadOnly = readOnly;
    }

    bool TextBox::IsReadOnly() const
    {
        return mReadOnly;
    }

    void TextBox::SetFocused(bool focused)
    {
        mFocused = focused;
    }

    bool TextBox::IsFocused() const
    {
        return mFocused;
    }

    void TextBox::SetMaxLength(std::size_t maxLength)
    {
        mMaxLength = maxLength;
        if (mText.size() > mMaxLength)
        {
            mText.resize(mMaxLength);
        }
    }

    std::size_t TextBox::GetMaxLength() const
    {
        return mMaxLength;
    }

    void TextBox::SetOnTextChanged(const std::function<void(const std::string&)>& callback)
    {
        mOnTextChanged = callback;
    }

    void TextBox::SetOnSubmit(const std::function<void(const std::string&)>& callback)
    {
        mOnSubmit = callback;
    }

    void TextBox::SetStyle(const Style& style)
    {
        mTextBoxStyle = style;
        Widget::SetStyle(style.baseStyle);
    }

    const TextBox::Style& TextBox::GetTextBoxStyle() const
    {
        return mTextBoxStyle;
    }
} // namespace nuvelocity
