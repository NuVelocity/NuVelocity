#include "Button.h"

#include "WidgetUtils.h"

#include "skin/JWindowSkin.h"
#include <system/FontManager.h>
#include <system/Game.h>
#include <system/InputManager.h>

#include <cctype>
#include <utility>

namespace nuvelocity
{
    namespace
    {
        SDL_Scancode MnemonicToScancode(char mnemonic)
        {
            const unsigned char upper =
                static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(mnemonic)));
            if (upper < 'A' || upper > 'Z')
            {
                return SDL_SCANCODE_UNKNOWN;
            }

            return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (upper - 'A'));
        }
    } // namespace

    Button::Button()
            : Button("")
    {
    }

    Button::Button(const std::string& caption)
            : mHovered(false)
            , mPressed(false)
            , mFocused(false)
            , mSuppressFocusAfterClick(false)
            , mMnemonicScancode(SDL_SCANCODE_UNKNOWN)
            , mMnemonicIndex(-1)

    {
        Widget::SetStyle(mButtonStyle.baseStyle);
        SetCaption(caption);
    }

    void Button::Update(Game* aGame)
    {
        if (aGame == nullptr || aGame->mInput == nullptr)
        {
            return;
        }

        if (!mVisible || !mEnabled)
        {
            mHovered = false;
            mPressed = false;
            return;
        }

        InputManager& input = *aGame->mInput;

        const SDL_Point mouse = input.GetMousePosition();
        mHovered = ContainsPoint(mouse);

        if (mHovered && input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            mPressed = true;
        }

        if (mPressed && input.IsMouseButtonReleased(SDL_BUTTON_LEFT))
        {
            const bool activate = mHovered;
            mPressed = false;
            if (activate && mOnClick)
            {
                mFocused = false;
                mSuppressFocusAfterClick = true;
                mOnClick(aGame);
            }
        }

        if (!input.IsMouseButtonDown(SDL_BUTTON_LEFT))
        {
            mPressed = false;
        }

        if (mFocused &&
            (input.IsKeyPressed(SDL_SCANCODE_RETURN) || input.IsKeyPressed(SDL_SCANCODE_SPACE)))
        {
            Activate(aGame);
        }

        const bool altDown =
            input.IsKeyDown(SDL_SCANCODE_LALT) || input.IsKeyDown(SDL_SCANCODE_RALT);
        if (altDown && mMnemonicScancode != SDL_SCANCODE_UNKNOWN &&
            input.IsKeyPressed(mMnemonicScancode))
        {
            Activate(aGame);
        }
    }

    void Button::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_Rect rect = GetScreenRect();

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawButton(game, this);
        }
    }

    void Button::SetCaption(const std::string& caption)
    {
        mCaption = caption;
        ParseCaption();
    }

    const std::string& Button::GetCaption() const
    {
        return mCaption;
    }

    const std::string& Button::GetDisplayCaption() const
    {
        return mDisplayCaption;
    }

    SDL_Scancode Button::GetMnemonicScancode() const
    {
        return mMnemonicScancode;
    }

    int Button::GetMnemonicIndex() const
    {
        return mMnemonicIndex;
    }

    void Button::SetFocused(bool focused)
    {
        if (!focused)
        {
            mFocused = false;
            mSuppressFocusAfterClick = false;
            return;
        }

        if (mSuppressFocusAfterClick)
        {
            mFocused = false;
            return;
        }

        mFocused = true;
    }

    bool Button::IsFocused() const
    {
        return mFocused;
    }

    void Button::SetOnClick(const std::function<void(Game*)>& callback)
    {
        mOnClick = callback;
    }

    void Button::Activate(Game* game)
    {
        if (mOnClick)
        {
            mOnClick(game);
        }
    }

    void Button::SetStyle(const Style& style)
    {
        mButtonStyle = style;
        Widget::SetStyle(style.baseStyle);
    }

    const Button::Style& Button::GetButtonStyle() const
    {
        return mButtonStyle;
    }

    void Button::ParseCaption()
    {
        mDisplayCaption.clear();
        mMnemonicScancode = SDL_SCANCODE_UNKNOWN;
        mMnemonicIndex = -1;

        bool mnemonicPending = false;
        for (char ch : mCaption)
        {
            if (ch == '_')
            {
                mnemonicPending = true;
                continue;
            }

            if (mnemonicPending && mMnemonicIndex < 0)
            {
                mMnemonicIndex = static_cast<int>(mDisplayCaption.size());
                mMnemonicScancode = MnemonicToScancode(ch);
            }

            mnemonicPending = false;
            mDisplayCaption.push_back(ch);
        }
    }

    bool Button::IsHovered() const
    {
        return mHovered;
    }

    bool Button::IsPressed() const
    {
        return mPressed;
    }
} // namespace nuvelocity
