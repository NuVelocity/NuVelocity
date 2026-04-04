#ifndef NVE_UI_TEXT_BOX_H
#define NVE_UI_TEXT_BOX_H

#include "Widget.h"

#include <functional>
#include <string>

namespace nuvelocity
{
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
            int fontPointSize = 12;
        };

        TextBox();

        void Update(InputManager& input, const SDL_FPoint& parentOffset) override;
        void Draw(Game* game, const SDL_FPoint& parentOffset) override;

        void SetText(const std::string& text);
        const std::string& GetText() const;

        void SetReadOnly(bool readOnly);
        bool IsReadOnly() const;

        void SetFocused(bool focused);
        bool IsFocused() const;

        void SetMaxLength(std::size_t maxLength);
        std::size_t GetMaxLength() const;

        void SetOnTextChanged(const std::function<void(const std::string&)>& callback);
        void SetOnSubmit(const std::function<void(const std::string&)>& callback);

        void SetStyle(const Style& style);
        const Style& GetTextBoxStyle() const;

    private:
        std::string mText;
        std::size_t mMaxLength;
        bool mReadOnly;
        bool mFocused;
        Style mTextBoxStyle;

        std::function<void(const std::string&)> mOnTextChanged;
        std::function<void(const std::string&)> mOnSubmit;
    };
} // namespace nuvelocity

#endif // NVE_UI_TEXT_BOX_H
