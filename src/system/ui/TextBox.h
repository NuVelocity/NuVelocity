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
