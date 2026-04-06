#include "Button.h"

#include "WidgetUtils.h"

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

    Button::Button(std::string caption)
            : mHovered(false)
            , mPressed(false)
            , mFocused(false)
            , mSuppressFocusAfterClick(false)
            , mMnemonicScancode(SDL_SCANCODE_UNKNOWN)
            , mMnemonicIndex(-1)
            , mCaption("")
    {
        Widget::SetStyle(mButtonStyle.baseStyle);
        SetCaption(caption);
    }

    void Button::Update(InputManager& input, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || !mEnabled)
        {
            mHovered = false;
            mPressed = false;
            return;
        }

        const SDL_FPoint mouse = input.GetMousePosition();
        mHovered = ContainsPoint(mouse, parentOffset);

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
                mOnClick();
            }
        }

        if (!input.IsMouseButtonDown(SDL_BUTTON_LEFT))
        {
            mPressed = false;
        }

        if (mFocused &&
            (input.IsKeyPressed(SDL_SCANCODE_RETURN) || input.IsKeyPressed(SDL_SCANCODE_SPACE)))
        {
            Activate();
        }

        const bool altDown =
            input.IsKeyDown(SDL_SCANCODE_LALT) || input.IsKeyDown(SDL_SCANCODE_RALT);
        if (altDown && mMnemonicScancode != SDL_SCANCODE_UNKNOWN &&
            input.IsKeyPressed(mMnemonicScancode))
        {
            Activate();
        }
    }

    void Button::Draw(Game* game, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_FRect rect = GetScreenRect(parentOffset);

        SDL_Color color = mStyle.backgroundColor;
        if (!mEnabled)
        {
            color = mStyle.disabledColor;
        }
        else if (mPressed)
        {
            color = mButtonStyle.pressedColor;
        }
        else if (mHovered)
        {
            color = mButtonStyle.hoverColor;
        }

        FillRect(game->mSpriteBatch, rect, color);
        DrawBevel(game->mSpriteBatch,
                  rect,
                  BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                  mPressed,
                  mStyle.borderThickness);

        SDL_FRect textRect{.x = rect.x + 4.0F,
                           .y = rect.y + 2.0F,
                           .w = SDL_max(0.0F, rect.w - 8.0F),
                           .h = SDL_max(0.0F, rect.h - 4.0F)};
        game->mFont->DrawString(game->mSpriteBatch,
                                mDisplayCaption,
                                textRect,
                                mButtonStyle.textColor,
                                mButtonStyle.fontPointSize,
                                TextAlignment::Center,
                                true,
                                mMnemonicIndex);

        if (mButtonStyle.showFocusRing && mFocused && !mHovered)
        {
            const SDL_FRect focusRect{.x = rect.x + 2.0F,
                                      .y = rect.y + 2.0F,
                                      .w = SDL_max(0.0F, rect.w - 4.0F),
                                      .h = SDL_max(0.0F, rect.h - 4.0F)};
            DrawRect(game->mSpriteBatch, focusRect, SDL_Color{255, 255, 255, 64});
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

    void Button::SetOnClick(const std::function<void()>& callback)
    {
        mOnClick = callback;
    }

    void Button::Activate()
    {
        if (mOnClick)
        {
            mOnClick();
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
} // namespace nuvelocity
