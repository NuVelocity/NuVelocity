#ifndef NVE_UI_WIDGET_H
#define NVE_UI_WIDGET_H

#include "API.h"
#include "Insets.h"
#include <SDL3/SDL.h>
#include <system/GameComponent.h>

#include <cstdint>

namespace nuvelocity
{
    class Game;
    class InputManager;
    class JWindowSkin;

    struct WidgetStyle
    {
        SDL_Color backgroundColor = SDL_Color{.r = 192, .g = 192, .b = 192, .a = 255};
        SDL_Color borderLightColor = SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255};
        SDL_Color borderDarkColor = SDL_Color{.r = 96, .g = 96, .b = 96, .a = 255};
        SDL_Color accentColor = SDL_Color{.r = 0, .g = 120, .b = 215, .a = 255};
        SDL_Color disabledColor = SDL_Color{.r = 128, .g = 128, .b = 128, .a = 255};
        int borderThickness = 1;
        Insets margin = {};
    };

    class Widget : public GameComponent
    {
    public:
        NVE_API Widget();
        NVE_API virtual ~Widget() = default;

        NVE_API virtual void Update(Game* game) override;
        NVE_API virtual void Draw(Game* game) override;

        NVE_API virtual void SetRect(const SDL_Rect& rect);
        NVE_API SDL_Rect GetRect() const;
        NVE_API SDL_Rect& GetActualRect();
        NVE_API SDL_Rect GetScreenRect() const;

        NVE_API void SetVisible(bool visible);
        NVE_API bool IsVisible() const;

        NVE_API void SetEnabled(bool enabled);
        NVE_API bool IsEnabled() const;

        NVE_API void SetStyle(const WidgetStyle& style);
        NVE_API const WidgetStyle& GetStyle() const;

        NVE_API void SetSkin(JWindowSkin* skin);
        NVE_API JWindowSkin* GetSkin(Game* game) const;

        NVE_API void SetParent(Widget* parent);
        NVE_API Widget* GetParent() const;

        NVE_API virtual SDL_Point GetContentOrigin() const;

    protected:
        NVE_API bool ContainsPoint(const SDL_Point& point) const;

        NVE_API virtual void InvalidateLayout();
        NVE_API virtual void DoLayout();

        SDL_Rect mRect;
        SDL_Rect mActualRect;
        WidgetStyle mStyle;
        bool mVisible;
        bool mEnabled;
        JWindowSkin* mSkin;
        Widget* mParent = nullptr;
        bool mNeedsLayout;
    };
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_H
