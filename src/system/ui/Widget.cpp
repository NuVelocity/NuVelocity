#include "Widget.h"

#include <system/Game.h>
#include <system/InputManager.h>
#include <system/ui/MdiManager.h>
#include <system/ui/skin/JWindowSkin.h>

namespace nuvelocity
{
    Widget::Widget()
            : mRect({.x = 0, .y = 0, .w = 0, .h = 0})
            , mActualRect({.x = 0, .y = 0, .w = 0, .h = 0})
            , mVisible(true)
            , mEnabled(true)
            , mSkin(nullptr)
            , mNeedsLayout(true)
    {
    }

    void Widget::Update(Game* game)
    {
        if (mNeedsLayout)
        {
            DoLayout();
            mNeedsLayout = false;
        }
    }

    void Widget::Draw(Game* game)
    {
        (void)game;
    }

    void Widget::SetRect(const SDL_Rect& rect)
    {
        mRect = rect;
        InvalidateLayout();
    }

    SDL_Rect Widget::GetRect() const
    {
        return mRect;
    }

    SDL_Rect& Widget::GetActualRect()
    {
        if (mNeedsLayout)
        {
            DoLayout();
            mNeedsLayout = false;
        }
        return mActualRect;
    }

    SDL_Rect Widget::GetScreenRect() const
    {
        if (mParent == nullptr)
        {
            return mRect;
        }

        const SDL_Point origin = mParent->GetContentOrigin();
        return SDL_Rect{
            .x = origin.x + mRect.x, .y = origin.y + mRect.y, .w = mRect.w, .h = mRect.h};
    }

    SDL_Point Widget::GetContentOrigin() const
    {
        const SDL_Rect rect = GetScreenRect();
        return SDL_Point{.x = rect.x, .y = rect.y};
    }

    void Widget::InvalidateLayout()
    {
        mNeedsLayout = true;
        if (mParent != nullptr)
        {
            mParent->InvalidateLayout();
        }
    }

    void Widget::DoLayout()
    {
        mActualRect = SDL_Rect{.x = mRect.x + mStyle.margin.left,
                               .y = mRect.y + mStyle.margin.top,
                               .w = mRect.w,
                               .h = mRect.h};
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
        InvalidateLayout();
    }

    const WidgetStyle& Widget::GetStyle() const
    {
        return mStyle;
    }

    bool Widget::ContainsPoint(const SDL_Point& point) const
    {
        SDL_Rect baseRect = GetScreenRect();
        SDL_Rect rect = SDL_Rect{.x = baseRect.x + mStyle.margin.left,
                                 .y = baseRect.y + mStyle.margin.top,
                                 .w = baseRect.w,
                                 .h = baseRect.h};
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
