#ifndef NVE_UI_BUTTON_H
#define NVE_UI_BUTTON_H

#include "Widget.h"

#include <functional>
#include <string>

namespace nuvelocity
{
    class Button : public Widget
    {
    public:
        struct Style
        {
            WidgetStyle baseStyle = WidgetStyle{};
            SDL_Color hoverColor = SDL_Color{.r = 208, .g = 208, .b = 208, .a = 255};
            SDL_Color pressedColor = SDL_Color{.r = 176, .g = 176, .b = 176, .a = 255};
            SDL_Color textColor = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 255};
            int fontPointSize = 12;
        };

        Button();
        explicit Button(std::string caption);

        void Update(InputManager& input, const SDL_FPoint& parentOffset) override;
        void Draw(Game* game, const SDL_FPoint& parentOffset) override;

        void SetCaption(const std::string& caption);
        const std::string& GetCaption() const;
        const std::string& GetDisplayCaption() const;
        SDL_Scancode GetMnemonicScancode() const;
        int GetMnemonicIndex() const;

        void SetFocused(bool focused);
        bool IsFocused() const;

        void SetOnClick(const std::function<void()>& callback);
        virtual void Activate();

        void SetStyle(const Style& style);
        const Style& GetButtonStyle() const;

    protected:
        bool mHovered;
        bool mPressed;
        bool mFocused;
        SDL_Scancode mMnemonicScancode;
        int mMnemonicIndex;
        std::string mCaption;
        std::string mDisplayCaption;
        Style mButtonStyle;
        std::function<void()> mOnClick;

    private:
        void ParseCaption();
    };
} // namespace nuvelocity

#endif // NVE_UI_BUTTON_H
