#ifndef NVE_UI_TEXT_BOX_H
#define NVE_UI_TEXT_BOX_H

#include "API.h"
#include "Widget.h"

#include <functional>
#include <string>

namespace nuvelocity
{
    class JWindowSkin;
    class TextBox : public Widget
    {
    public:
        struct Style
        {
            WidgetStyle baseStyle = WidgetStyle{};
            SDL_Color focusedColor = SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255};
            SDL_Color unfocusedColor = SDL_Color{.r = 236, .g = 236, .b = 236, .a = 255};
            SDL_Color caretColor = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 255};
            SDL_Color textColor = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 255};
            SDL_Color selectionColor = SDL_Color{.r = 0, .g = 120, .b = 215, .a = 128};
            int fontPointSize = 12;
        };

        NVE_API TextBox();

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void SetText(const std::string& text);
        NVE_API const std::string& GetText() const;

        NVE_API void SetReadOnly(bool readOnly);
        NVE_API bool IsReadOnly() const;

        NVE_API void SetFocused(bool focused);
        NVE_API bool IsFocused() const;

        NVE_API void SetMaxLength(std::size_t maxLength);
        NVE_API std::size_t GetMaxLength() const;

        NVE_API void SetOnTextChanged(const std::function<void(const std::string&)>& callback);
        NVE_API void SetOnSubmit(const std::function<void(const std::string&)>& callback);

        NVE_API void SetStyle(const Style& style);
        NVE_API const Style& GetTextBoxStyle() const;

        // Caret
        NVE_API std::size_t GetCaretPos() const;
        NVE_API void SetCaretPos(std::size_t pos);

        // Selection — selStart <= selEnd, both byte offsets into mText
        NVE_API bool HasSelection() const;
        NVE_API void GetSelection(std::size_t& start, std::size_t& end) const;
        NVE_API void SelectAll();
        NVE_API void ClearSelection();

        // Overwrite (Insert key)
        NVE_API bool IsOverwriteMode() const;

        // Blink — skin calls this so caret visibility is centralised here
        NVE_API bool IsCaretVisible() const;

        // Horizontal scroll offset in pixels (how many pixels the text is shifted left)
        NVE_API int GetScrollOffset() const;

    private:
        std::string mText;
        std::size_t mMaxLength;
        bool mReadOnly;
        bool mFocused;
        Style mTextBoxStyle;

        // Caret & selection state
        std::size_t mCaretPos;     // byte offset in mText; caret is before this index
        std::size_t mSelAnchor;    // anchor set when selection starts (byte offset)
        std::size_t mSelEnd;       // current drag/extend end (byte offset)
        bool mHasSelection;        // true when mSelAnchor != mSelEnd
        bool mOverwriteMode;       // toggled by Insert key
        bool mDragging;            // true while mouse button held for drag-select
        uint64_t mCaretBlinkStart; // SDL_GetTicks() at last caret reset; drives blink
        int mScrollOffset;         // pixels the text is scrolled left to keep caret visible

        std::function<void(const std::string&)> mOnTextChanged;
        std::function<void(const std::string&)> mOnSubmit;

        // Returns byte offset of the caret position closest to the given pixel X
        // in text-space (i.e. already adjusted for mScrollOffset by the caller).
        std::size_t HitTestCaret(Game* game, int pixelX) const;

        // Deletes [selStart, selEnd) from mText; moves caret to selStart; clears selection.
        void DeleteSelection();

        // Resets blink timer; call on any input that moves the caret.
        void ResetBlink();

        // Clamps mCaretPos to [0, mText.size()]; normalises selection range.
        void ClampCaret();

        // Adjusts mScrollOffset so the caret is within the visible text area.
        void EnsureCaretVisible(Game* game);
    };
} // namespace nuvelocity

#endif // NVE_UI_TEXT_BOX_H
