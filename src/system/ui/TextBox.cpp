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
            , mCaretPos(0)
            , mSelAnchor(0)
            , mSelEnd(0)
            , mHasSelection(false)
            , mOverwriteMode(false)
            , mDragging(false)
            , mCaretBlinkStart(0)
            , mScrollOffset(0)
    {
        Widget::SetStyle(mTextBoxStyle.baseStyle);
    }

    void TextBox::ResetBlink()
    {
        mCaretBlinkStart = SDL_GetTicks();
    }

    void TextBox::ClampCaret()
    {
        const std::size_t len = mText.size();
        if (mCaretPos > len)
        {
            mCaretPos = len;
        }
        if (mSelAnchor > len)
        {
            mSelAnchor = len;
        }
        if (mSelEnd > len)
        {
            mSelEnd = len;
        }
    }

    void TextBox::DeleteSelection()
    {
        if (!mHasSelection)
        {
            return;
        }
        const std::size_t lo = (mSelAnchor < mSelEnd) ? mSelAnchor : mSelEnd;
        const std::size_t hi = (mSelAnchor < mSelEnd) ? mSelEnd : mSelAnchor;
        mText.erase(lo, hi - lo);
        mCaretPos = lo;
        mSelAnchor = lo;
        mSelEnd = lo;
        mHasSelection = false;
    }

    std::size_t TextBox::HitTestCaret(Game* game, int pixelX) const
    {
        JWindowSkin* skin = GetSkin(game);
        if (skin == nullptr || game == nullptr || game->mFont == nullptr)
        {
            return mCaretPos;
        }

        const int pointSize = mTextBoxStyle.fontPointSize;
        int prevWidth = 0;
        for (std::size_t i = 0; i < mText.size(); ++i)
        {
            const int nextWidth = skin->MeasureTextWidth(game, mText.substr(0, i + 1), pointSize);
            // Snap to nearest half-character boundary
            if (pixelX < (prevWidth + nextWidth) / 2)
            {
                return i;
            }
            prevWidth = nextWidth;
        }
        return mText.size();
    }

    std::size_t TextBox::GetCaretPos() const
    {
        return mCaretPos;
    }

    void TextBox::SetCaretPos(std::size_t pos)
    {
        mCaretPos = pos;
        ClampCaret();
        ResetBlink();
    }

    bool TextBox::HasSelection() const
    {
        return mHasSelection;
    }

    void TextBox::GetSelection(std::size_t& start, std::size_t& end) const
    {
        if (mSelAnchor <= mSelEnd)
        {
            start = mSelAnchor;
            end = mSelEnd;
        }
        else
        {
            start = mSelEnd;
            end = mSelAnchor;
        }
    }

    void TextBox::SelectAll()
    {
        mSelAnchor = 0;
        mSelEnd = mText.size();
        mCaretPos = mSelEnd;
        mHasSelection = (mSelAnchor != mSelEnd);
        ResetBlink();
    }

    void TextBox::ClearSelection()
    {
        mSelAnchor = mCaretPos;
        mSelEnd = mCaretPos;
        mHasSelection = false;
    }

    bool TextBox::IsOverwriteMode() const
    {
        return mOverwriteMode;
    }

    bool TextBox::IsCaretVisible() const
    {
        // Blink: on for 500 ms, off for 500 ms, anchored to last reset
        const uint64_t elapsed = SDL_GetTicks() - mCaretBlinkStart;
        return ((elapsed / 500U) % 2U) == 0U;
    }

    int TextBox::GetScrollOffset() const
    {
        return mScrollOffset;
    }

    void TextBox::EnsureCaretVisible(Game* game)
    {
        JWindowSkin* skin = GetSkin(game);
        if (skin == nullptr)
        {
            return;
        }

        // Text area width matches the textRect.w used in both skins (rect.w - 12)
        const SDL_Rect screenRect = GetScreenRect();
        const int textAreaW = SDL_max(1, screenRect.w - 12);

        const int caretPixelX =
            skin->MeasureTextWidth(game, mText.substr(0, mCaretPos), mTextBoxStyle.fontPointSize);

        if (caretPixelX - mScrollOffset < 0)
        {
            // Caret is left of visible area — scroll to show it
            mScrollOffset = caretPixelX;
        }
        else if (caretPixelX - mScrollOffset > textAreaW - 1)
        {
            // Caret is right of visible area — scroll right
            mScrollOffset = caretPixelX - (textAreaW - 1);
        }

        // Clamp lower bound
        if (mScrollOffset < 0)
        {
            mScrollOffset = 0;
        }
    }

    void TextBox::Update(Game* game)
    {
        if (game == nullptr || game->mInput == nullptr)
        {
            return;
        }

        if (!mVisible || !mEnabled)
        {
            return;
        }

        InputManager& input = *game->mInput;

        // Mouse: focus, click-to-place caret, drag to select
        const SDL_Point mouse = input.GetMousePosition();
        const SDL_Rect screenRect = GetScreenRect();
        // Text area origin (same padding used in Draw)
        const int textAreaX = screenRect.x + 6;

        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            const bool wasFocused = mFocused;
            mFocused = ContainsPoint(mouse);

            if (mFocused)
            {
                if (!wasFocused)
                {
                    SDL_StartTextInput(game->mWindow);
                }
                // Place caret at click position (relX in text-pixel space = screen offset + scroll)
                const int relX = mouse.x - textAreaX + mScrollOffset;
                mCaretPos = HitTestCaret(game, relX);
                mSelAnchor = mCaretPos;
                mSelEnd = mCaretPos;
                mHasSelection = false;
                mDragging = true;
                ResetBlink();
                EnsureCaretVisible(game);
            }
            else
            {
                if (wasFocused)
                {
                    SDL_StopTextInput(game->mWindow);
                }
                mDragging = false;
            }
        }

        if (input.IsMouseButtonReleased(SDL_BUTTON_LEFT))
        {
            mDragging = false;
        }

        if (mDragging && input.IsMouseButtonDown(SDL_BUTTON_LEFT) && mFocused)
        {
            const int relX = mouse.x - textAreaX + mScrollOffset;
            const std::size_t dragCaret = HitTestCaret(game, relX);
            mSelEnd = dragCaret;
            mCaretPos = dragCaret;
            mHasSelection = (mSelAnchor != mSelEnd);
            EnsureCaretVisible(game);
        }

        if (!mFocused || mReadOnly)
        {
            return;
        }

        // Ensure text input is active if we are focused (e.g. set via SetFocused)
        if (!SDL_TextInputActive(game->mWindow))
        {
            SDL_StartTextInput(game->mWindow);
        }

        // Keyboard: navigation, editing, clipboard
        const bool ctrlHeld =
            input.IsKeyDown(SDL_SCANCODE_LCTRL) || input.IsKeyDown(SDL_SCANCODE_RCTRL);
        const bool shiftHeld =
            input.IsKeyDown(SDL_SCANCODE_LSHIFT) || input.IsKeyDown(SDL_SCANCODE_RSHIFT);

        bool changed = false;

        // Iterate raw frame events so we can act on SDL_EVENT_KEY_DOWN
        for (const SDL_Event& ev : input.GetFrameEvents())
        {
            if (ev.type != SDL_EVENT_KEY_DOWN)
            {
                continue;
            }

            const SDL_Scancode sc = ev.key.scancode;

            // Navigation / selection keys

            if (sc == SDL_SCANCODE_LEFT)
            {
                if (shiftHeld)
                {
                    // Extend selection left
                    if (!mHasSelection)
                    {
                        mSelAnchor = mCaretPos;
                    }
                    if (mCaretPos > 0)
                    {
                        mCaretPos -= 1;
                    }
                    mSelEnd = mCaretPos;
                    mHasSelection = (mSelAnchor != mSelEnd);
                }
                else
                {
                    if (mHasSelection)
                    {
                        // Jump to selection start
                        std::size_t lo = 0;
                        std::size_t hi = 0;
                        GetSelection(lo, hi);
                        mCaretPos = lo;
                    }
                    else if (mCaretPos > 0)
                    {
                        mCaretPos -= 1;
                    }
                    ClearSelection();
                }
                ResetBlink();
                continue;
            }

            if (sc == SDL_SCANCODE_RIGHT)
            {
                if (shiftHeld)
                {
                    if (!mHasSelection)
                    {
                        mSelAnchor = mCaretPos;
                    }
                    if (mCaretPos < mText.size())
                    {
                        mCaretPos += 1;
                    }
                    mSelEnd = mCaretPos;
                    mHasSelection = (mSelAnchor != mSelEnd);
                }
                else
                {
                    if (mHasSelection)
                    {
                        std::size_t lo = 0;
                        std::size_t hi = 0;
                        GetSelection(lo, hi);
                        mCaretPos = hi;
                    }
                    else if (mCaretPos < mText.size())
                    {
                        mCaretPos += 1;
                    }
                    ClearSelection();
                }
                ResetBlink();
                continue;
            }

            if (sc == SDL_SCANCODE_HOME)
            {
                if (shiftHeld)
                {
                    if (!mHasSelection)
                    {
                        mSelAnchor = mCaretPos;
                    }
                    mCaretPos = 0;
                    mSelEnd = mCaretPos;
                    mHasSelection = (mSelAnchor != mSelEnd);
                }
                else
                {
                    mCaretPos = 0;
                    ClearSelection();
                }
                ResetBlink();
                continue;
            }

            if (sc == SDL_SCANCODE_END)
            {
                if (shiftHeld)
                {
                    if (!mHasSelection)
                    {
                        mSelAnchor = mCaretPos;
                    }
                    mCaretPos = mText.size();
                    mSelEnd = mCaretPos;
                    mHasSelection = (mSelAnchor != mSelEnd);
                }
                else
                {
                    mCaretPos = mText.size();
                    ClearSelection();
                }
                ResetBlink();
                continue;
            }

            // Insert (overwrite toggle)

            if (sc == SDL_SCANCODE_INSERT && !ctrlHeld && !shiftHeld)
            {
                mOverwriteMode = !mOverwriteMode;
                continue;
            }

            // Ctrl shortcuts

            if (ctrlHeld)
            {
                if (sc == SDL_SCANCODE_A)
                {
                    SelectAll();
                    continue;
                }

                if (sc == SDL_SCANCODE_C || sc == SDL_SCANCODE_X)
                {
                    if (mHasSelection)
                    {
                        std::size_t lo = 0;
                        std::size_t hi = 0;
                        GetSelection(lo, hi);
                        const std::string selected = mText.substr(lo, hi - lo);
                        SDL_SetClipboardText(selected.c_str());

                        if (sc == SDL_SCANCODE_X)
                        {
                            DeleteSelection();
                            changed = true;
                        }
                    }
                    continue;
                }

                if (sc == SDL_SCANCODE_V)
                {
                    if (SDL_HasClipboardText())
                    {
                        char* raw = SDL_GetClipboardText();
                        if (raw != nullptr)
                        {
                            if (mHasSelection)
                            {
                                DeleteSelection();
                            }
                            for (char ch : std::string(raw))
                            {
                                if (ch < 32 || ch > 126)
                                {
                                    continue;
                                }
                                if (mText.size() >= mMaxLength)
                                {
                                    break;
                                }
                                mText.insert(mCaretPos, 1, ch);
                                mCaretPos += 1;
                            }
                            SDL_free(raw);
                            changed = true;
                            ResetBlink();
                        }
                    }
                    continue;
                }

                // Other ctrl combos fall through (e.g. do nothing)
                continue;
            }

            // Backspace

            if (sc == SDL_SCANCODE_BACKSPACE)
            {
                if (mHasSelection)
                {
                    DeleteSelection();
                    changed = true;
                }
                else if (mCaretPos > 0)
                {
                    mText.erase(mCaretPos - 1, 1);
                    mCaretPos -= 1;
                    changed = true;
                }
                ResetBlink();
                continue;
            }

            // Delete

            if (sc == SDL_SCANCODE_DELETE)
            {
                if (mHasSelection)
                {
                    DeleteSelection();
                    changed = true;
                }
                else if (mCaretPos < mText.size())
                {
                    mText.erase(mCaretPos, 1);
                    changed = true;
                }
                ResetBlink();
                continue;
            }

            // Return / Enter

            if (sc == SDL_SCANCODE_RETURN)
            {
                if (mOnSubmit)
                {
                    mOnSubmit(mText);
                }
                continue;
            }
        }

        // Text input (printable characters)
        const std::string& textInput = input.GetTextInput();
        for (char ch : textInput)
        {
            if (ch < 32 || ch > 126)
            {
                continue;
            }
            if (mText.size() >= mMaxLength && !mHasSelection)
            {
                break;
            }

            if (mHasSelection)
            {
                DeleteSelection();
            }

            if (mOverwriteMode && mCaretPos < mText.size())
            {
                mText[mCaretPos] = ch;
                mCaretPos += 1;
            }
            else
            {
                if (mText.size() < mMaxLength)
                {
                    mText.insert(mCaretPos, 1, ch);
                    mCaretPos += 1;
                }
            }
            changed = true;
            ResetBlink();
        }

        if (changed && mOnTextChanged)
        {
            mOnTextChanged(mText);
        }

        EnsureCaretVisible(game);
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
        mCaretPos = mText.size();
        mSelAnchor = mCaretPos;
        mSelEnd = mCaretPos;
        mHasSelection = false;
        mScrollOffset = 0;
        ResetBlink();
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
        ClampCaret();
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
