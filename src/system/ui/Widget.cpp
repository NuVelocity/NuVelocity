#include "Widget.h"

#include <system/Game.h>
#include <system/InputManager.h>
#include <system/ui/MdiManager.h>
#include <system/ui/skin/JWindowSkin.h>

namespace nuvelocity
{
    Widget::Widget()
            : mRect({.x = 0, .y = 0, .w = 0, .h = 0})
            , mVisible(true)
            , mEnabled(true)
            , mSkin(nullptr)
    {
    }

    void Widget::SetRect(const SDL_Rect& rect)
    {
        mRect = rect;
    }

    SDL_Rect Widget::GetRect() const
    {
        return mRect;
    }

    SDL_Rect Widget::GetScreenRect() const
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

    bool Widget::ContainsPoint(const SDL_Point& point) const
    {
        const SDL_Rect rect = GetScreenRect();
        return point.x >= rect.x && point.y >= rect.y && point.x <= rect.x + rect.w &&
               point.y <= rect.y + rect.h;
    }

    void Widget::SetSkin(JWindowSkin* skin)
    {
        mSkin = skin;
    }

    JWindowSkin* Widget::GetSkin(Game* game) const
    {
        if (mSkin != nullptr)
        {
            return mSkin;
        }

        if (game != nullptr && game->mMdi != nullptr)
        {
            return game->mMdi->GetDefaultSkin();
        }

        return nullptr;
    }

    void Widget::SetParent(Widget* parent)
    {
        mParent = parent;
    }

    Widget* Widget::GetParent() const
    {
        return mParent;
    }
} // namespace nuvelocity
