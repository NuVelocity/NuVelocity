#include "Widget.h"

#include <system/Game.h>
#include <system/InputManager.h>

namespace nuvelocity
{
    Widget::Widget()
            : mRect({.x = 0.0F, .y = 0.0F, .w = 0.0F, .h = 0.0F})
            , mVisible(true)
            , mEnabled(true)
    {
    }

    void Widget::SetRect(const SDL_FRect& rect)
    {
        mRect = rect;
    }

    SDL_FRect Widget::GetRect() const
    {
        return mRect;
    }

    SDL_FRect Widget::GetScreenRect() const
    {
        return mRect;
    }

    void Widget::SetVisible(bool visible)
    {
        mVisible = visible;
    }

    bool Widget::IsVisible() const
    {
        return mVisible;
    }

    void Widget::SetEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    bool Widget::IsEnabled() const
    {
        return mEnabled;
    }

    void Widget::SetStyle(const WidgetStyle& style)
    {
        mStyle = style;
    }

    const WidgetStyle& Widget::GetStyle() const
    {
        return mStyle;
    }

    bool Widget::ContainsPoint(const SDL_FPoint& point) const
    {
        const SDL_FRect rect = GetScreenRect();
        return point.x >= rect.x && point.y >= rect.y && point.x <= rect.x + rect.w &&
               point.y <= rect.y + rect.h;
    }
} // namespace nuvelocity
