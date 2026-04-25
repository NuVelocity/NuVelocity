#ifndef NVE_UI_WIDGET_H
#define NVE_UI_WIDGET_H

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
    };

    class Widget : public GameComponent
    {
    public:
        Widget();
        virtual ~Widget() = default;

        void Update(Game* game) override = 0;
        void Draw(Game* game) override = 0;

        virtual void SetRect(const SDL_Rect& rect);
        SDL_Rect GetRect() const;
        SDL_Rect GetScreenRect() const;

        void SetVisible(bool visible);
        bool IsVisible() const;

        void SetEnabled(bool enabled);
        bool IsEnabled() const;

        void SetStyle(const WidgetStyle& style);
        const WidgetStyle& GetStyle() const;

        void SetSkin(JWindowSkin* skin);
        JWindowSkin* GetSkin(Game* game) const;

        void SetParent(Widget* parent);
        Widget* GetParent() const;

    protected:
        bool ContainsPoint(const SDL_Point& point) const;

        SDL_Rect mRect;
        WidgetStyle mStyle;
        bool mVisible;
        bool mEnabled;
        JWindowSkin* mSkin;
        Widget* mParent = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_H
