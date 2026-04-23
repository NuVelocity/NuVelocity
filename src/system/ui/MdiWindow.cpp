#include "MdiWindow.h"
#include "skin/JWindowSkin.h"
#include <StandAloneFrame.h>
#include <system/FontManager.h>
#include <system/Game.h>
#include <system/InputManager.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    MdiWindow::MdiWindow()
            : MdiWindow("")
    {
    }

    MdiWindow::MdiWindow(std::string title)
            : mTitle(std::move(title))
            , mMovable(true)
            , mClosable(true)
            , mActive(false)
            , mDragging(false)
            , mShouldClose(false)
            , mDragGrabOffset({.x = 0.0F, .y = 0.0F})
    {
        Widget::SetStyle(mWindowStyle.baseStyle);
    }

    void MdiWindow::Update(Game* aGame)
    {
        if (aGame == nullptr || aGame->mInput == nullptr)
        {
            return;
        }

        if (!mVisible || !mEnabled)
        {
            return;
        }

        InputManager& input = *aGame->mInput;
        const SDL_FPoint mouse = input.GetMousePosition();
        JWindowSkin* skin = GetSkin(aGame);

        SDL_FRect titleBar = skin == nullptr ? GetTitleBarRect() : skin->GetInnerRect(this);
        SDL_FRect closeButton =
            skin == nullptr ? GetCloseButtonRect() : skin->GetCloseButtonRect(this);

        const bool overTitle = mouse.x >= titleBar.x && mouse.y >= titleBar.y &&
                               mouse.x <= titleBar.x + titleBar.w &&
                               mouse.y <= titleBar.y + titleBar.h;
        const bool overClose = mouse.x >= closeButton.x && mouse.y >= closeButton.y &&
                               mouse.x <= closeButton.x + closeButton.w &&
                               mouse.y <= closeButton.y + closeButton.h;

        if (mClosable && overClose && input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            mShouldClose = true;
            if (mOnClose)
            {
                mOnClose(*this);
            }
            return;
        }

        if (mMovable && overTitle && input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            mDragging = true;
            const SDL_FRect screenRect = GetScreenRect();
            mDragGrabOffset = SDL_FPoint{.x = mouse.x - screenRect.x, .y = mouse.y - screenRect.y};
        }

        if (mDragging)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                const SDL_FRect newRect{.x = mouse.x - mDragGrabOffset.x,
                                        .y = mouse.y - mDragGrabOffset.y,
                                        .w = mRect.w,
                                        .h = mRect.h};
                SetRect(newRect);
            }
            else
            {
                mDragging = false;
            }
        }

        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Update(aGame);
            }
        }

        if (mAutoResize && !mChildren.empty())
        {
            float maxBottom = 0;
            for (const auto& child : mChildren)
            {
                if (child->IsVisible())
                {
                    const SDL_FRect r = child->GetRect();
                    maxBottom = SDL_max(maxBottom, r.y + r.h);
                }
            }

            if (maxBottom > 0)
            {
                const SDL_FRect clientRect = GetClientRect();
                const float bottomChrome = mRect.y + mRect.h - (clientRect.y + clientRect.h);
                const float newHeight = (maxBottom - mRect.y) + bottomChrome;

                if (std::abs(mRect.h - newHeight) > 0.5f)
                {
                    SDL_FRect newRect = mRect;
                    newRect.h = newHeight;
                    SetRect(newRect);
                }
            }
        }

        if (mAutoCenter && aGame != nullptr)
        {
            const float centerX = (static_cast<float>(aGame->mWindowWidth) - mRect.w) * 0.5f;
            const float centerY = (static_cast<float>(aGame->mWindowHeight) - mRect.h) * 0.5f;

            if (SDL_fabs(mRect.x - centerX) > 0.5f || SDL_fabs(mRect.y - centerY) > 0.5f)
            {
                SetRect({centerX, centerY, mRect.w, mRect.h});
            }
        }
    }

    void MdiWindow::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_FRect windowRect = GetScreenRect();
        const SDL_Rect clipRect = {static_cast<int>(windowRect.x),
                                   static_cast<int>(windowRect.y),
                                   static_cast<int>(windowRect.w),
                                   static_cast<int>(windowRect.h)};
        // game->mSpriteBatch->SetClipRect(&clipRect);

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawMdiWindow(game, this);
        }

        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Draw(game);
            }
        }

        if (game->mSpriteBatch->IsDrawBoundsEnabled())
        {
            game->mSpriteBatch->OutlineRect(&windowRect, SDL_Color{255, 0, 0, 255}, 1.0F);
        }

        game->mSpriteBatch->SetClipRect(nullptr);
    }

    void MdiWindow::SetTitle(const std::string& title)
    {
        mTitle = title;
    }

    const std::string& MdiWindow::GetTitle() const
    {
        return mTitle;
    }

    void MdiWindow::SetMovable(bool movable)
    {
        mMovable = movable;
    }

    bool MdiWindow::IsMovable() const
    {
        return mMovable;
    }

    void MdiWindow::SetClosable(bool closable)
    {
        mClosable = closable;
    }

    bool MdiWindow::IsClosable() const
    {
        return mClosable;
    }

    void MdiWindow::SetActive(bool active)
    {
        mActive = active;
    }

    bool MdiWindow::IsActive() const
    {
        return mActive;
    }

    void MdiWindow::SetBackgroundTile(StandAloneFrame* frame)
    {
        mBackgroundTile = frame;
    }

    StandAloneFrame* MdiWindow::GetBackgroundTile() const
    {
        return mBackgroundTile;
    }

    void MdiWindow::SetStyle(const Style& style)
    {
        mWindowStyle = style;
        Widget::SetStyle(style.baseStyle);
    }

    const MdiWindow::Style& MdiWindow::GetWindowStyle() const
    {
        return mWindowStyle;
    }

    void MdiWindow::AddChild(const std::shared_ptr<Widget>& widget)
    {
        if (widget != nullptr)
        {
            const SDL_FRect clientRect = GetClientRect();
            SDL_FRect childRect = widget->GetRect();
            childRect.x += clientRect.x;
            childRect.y += clientRect.y;
            widget->SetRect(childRect);
            widget->SetParent(this);

            mChildren.push_back(widget);
        }
    }

    void MdiWindow::SetRect(const SDL_FRect& rect)
    {
        const float deltaX = rect.x - mRect.x;
        const float deltaY = rect.y - mRect.y;

        mRect = rect;

        for (const auto& child : mChildren)
        {
            if (child != nullptr)
            {
                SDL_FRect r = child->GetRect();
                r.x += deltaX;
                r.y += deltaY;
                child->SetRect(r);
            }
        }
    }

    const std::vector<std::shared_ptr<Widget>>& MdiWindow::GetChildren() const
    {
        return mChildren;
    }

    void MdiWindow::SetAutoResize(bool autoResize)
    {
        mAutoResize = autoResize;
    }

    bool MdiWindow::IsAutoResize() const
    {
        return mAutoResize;
    }

    void MdiWindow::SetAutoCenter(bool autoCenter)
    {
        mAutoCenter = autoCenter;
    }

    bool MdiWindow::IsAutoCenter() const
    {
        return mAutoCenter;
    }

    void MdiWindow::Close()
    {
        mShouldClose = true;
        if (mOnClose)
        {
            mOnClose(*this);
        }
    }

    bool MdiWindow::ShouldClose() const
    {
        return mShouldClose;
    }

    bool MdiWindow::Intersects(const SDL_FPoint& point) const
    {
        return point.x >= mRect.x && point.y >= mRect.y && point.x <= mRect.x + mRect.w &&
               point.y <= mRect.y + mRect.h;
    }

    void MdiWindow::SetOnClose(const std::function<void(MdiWindow&)>& callback)
    {
        mOnClose = callback;
    }

    SDL_FRect MdiWindow::GetTitleBarRect() const
    {
        const SDL_FRect rect = GetScreenRect();
        return SDL_FRect{.x = rect.x,
                         .y = rect.y,
                         .w = rect.w,
                         .h = SDL_min(rect.h, mWindowStyle.titleBarHeight)};
    }

    SDL_FRect MdiWindow::GetCloseButtonRect() const
    {
        const SDL_FRect titleBar = GetTitleBarRect();
        const float margin = (titleBar.h - mWindowStyle.closeButtonSize) * 0.5F;

        return SDL_FRect{.x = titleBar.x + titleBar.w - mWindowStyle.closeButtonSize - margin,
                         .y = titleBar.y + margin,
                         .w = mWindowStyle.closeButtonSize,
                         .h = mWindowStyle.closeButtonSize};
    }

    SDL_FRect MdiWindow::GetClientRect() const
    {
        if (mSkin != nullptr)
        {
            return mSkin->GetInnerRect(this);
        }

        const SDL_FRect rect = GetScreenRect();

        const float inset = mWindowStyle.borderSize;
        const float topInset = mWindowStyle.titleBarHeight;

        return SDL_FRect{.x = rect.x + inset,
                         .y = rect.y + topInset,
                         .w = SDL_max(0.0F, rect.w - (inset * 2.0F)),
                         .h = SDL_max(0.0F, rect.h - topInset - inset)};
    }
} // namespace nuvelocity
