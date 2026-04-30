#ifndef NVE_UI_BUTTON_H
#define NVE_UI_BUTTON_H

#include "API.h"
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
            bool showFocusRing = true;
        };

        NVE_API Button();
        NVE_API explicit Button(const std::string& caption);

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void SetCaption(const std::string& caption);
        NVE_API const std::string& GetCaption() const;
        NVE_API const std::string& GetDisplayCaption() const;
        NVE_API SDL_Scancode GetMnemonicScancode() const;
        NVE_API int GetMnemonicIndex() const;

        NVE_API void SetFocused(bool focused);
        NVE_API bool IsFocused() const;

        NVE_API void SetOnClick(const std::function<void(Game*)>& callback);
        NVE_API virtual void Activate(Game* game);

        NVE_API void SetStyle(const Style& style);
        NVE_API const Style& GetButtonStyle() const;

        NVE_API bool IsHovered() const;
        NVE_API bool IsPressed() const;

    protected:
        bool mHovered;
        bool mPressed;
        bool mFocused;
        bool mSuppressFocusAfterClick;
        SDL_Scancode mMnemonicScancode;
        int mMnemonicIndex;
        std::string mCaption;
        std::string mDisplayCaption;
        Style mButtonStyle;
        std::function<void(Game*)> mOnClick;

    private:
        void ParseCaption();
    };
} // namespace nuvelocity

#endif // NVE_UI_BUTTON_H
