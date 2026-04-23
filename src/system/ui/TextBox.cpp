#include "TextBox.h"
#include "WidgetUtils.h"
#include "skin/JWindowSkin.h"

#include <SDL3/SDL.h>
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

    void TextBox::Update(Game* aGame)
    {
        if (aGame == nullptr || aGame->mInput == nullptr)
        {
            return;
        }

        if (!mVisible || !mEnabled)
        {
            return;
        }

        InputManager& input = *aGame->mInput;

        const SDL_Point mouse = input.GetMousePosition();
        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            const bool wasFocused = mFocused;
            mFocused = ContainsPoint(mouse);
            if (mFocused && !wasFocused)
            {
                SDL_StartTextInput(aGame->mWindow);
            }
            else if (!mFocused && wasFocused)
            {
                SDL_StopTextInput(aGame->mWindow);
            }
        }

        if (!mFocused || mReadOnly)
        {
            return;
        }

        // Ensure text input is active if we are focused (e.g., set via SetFocused)
        if (!SDL_TextInputActive(aGame->mWindow))
        {
            SDL_StartTextInput(aGame->mWindow);
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

    void TextBox::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawTextBox(game, this);
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
