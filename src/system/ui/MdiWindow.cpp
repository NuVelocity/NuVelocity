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
            , mDragGrabOffset({.x = 0, .y = 0})
    {
        Widget::SetStyle(mWindowStyle.baseStyle);
    }

    void MdiWindow::Update(Game* game)
    {
        if (game == nullptr || game->mInput == nullptr)
        {
            return;
        }

        if (!mVisible || !mEnabled)
        {
            return;
        }

        InputManager& input = *game->mInput;
        const SDL_Point mouse = input.GetMousePosition();
        JWindowSkin* skin = GetSkin(game);

        SDL_Rect titleBar = skin == nullptr ? GetTitleBarRect() : skin->GetInnerRect(this);
        SDL_Rect closeButton =
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
            const SDL_Rect screenRect = GetScreenRect();
            mDragGrabOffset = SDL_Point{.x = mouse.x - screenRect.x, .y = mouse.y - screenRect.y};
        }

        if (mDragging)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                const SDL_Rect newRect{.x = mouse.x - mDragGrabOffset.x,
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
                child->Update(game);
            }
        }

        if (mFullScreen && game != nullptr)
        {
            if (mRect.x != 0 || mRect.y != 0 || mRect.w != game->mWindowWidth ||
                mRect.h != game->mWindowHeight)
            {
                SetRect({.x = 0, .y = 0, .w = game->mWindowWidth, .h = game->mWindowHeight});
            }
            mDragging = false;
        }
    }

    void MdiWindow::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_Rect windowRect = GetScreenRect();
        const SDL_Rect clipRect = {
            .x = windowRect.x, .y = windowRect.y, .w = windowRect.w, .h = windowRect.h};
#if !DEBUG
        game->mSpriteBatch->SetClipRect(&clipRect);
#endif
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
#if !DEBUG
        game->mSpriteBatch->SetClipRect(nullptr);
#endif
        if (game->mSpriteBatch->IsDrawBoundsEnabled())
        {
            const SDL_Rect clientRect = GetClientRect();
            game->mSpriteBatch->OutlineRect(&clientRect, Colors::Red);
        }
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
            widget->SetParent(this);
            mChildren.push_back(widget);
        }
    }

    void MdiWindow::SetRect(const SDL_Rect& rect)
    {
        mRect = rect;
    }

    SDL_Point MdiWindow::GetContentOrigin() const
    {
        const SDL_Rect clientRect = GetClientRect();
        return SDL_Point{.x = clientRect.x, .y = clientRect.y};
    }

    const std::vector<std::shared_ptr<Widget>>& MdiWindow::GetChildren() const
    {
        return mChildren;
    }

    void MdiWindow::FitToChildren(Game* game)
    {
        // First update children to ensure their sizes are correct before fitting.
        Update(game);

        int maxBottom = 0;

        for (const auto& child : mChildren)
        {
            if (child->IsVisible())
            {
                const SDL_Rect r = child->GetRect();
                maxBottom = SDL_max(maxBottom, r.y + r.h);
            }
        }

        if (maxBottom > 0)
        {
            const SDL_Rect windowRect = GetScreenRect();
            const SDL_Rect clientRect = GetClientRect();
            const int topInset = clientRect.y - windowRect.y;
            const int bottomInset = (windowRect.y + windowRect.h) - (clientRect.y + clientRect.h);
            const int newHeight = topInset + maxBottom + bottomInset;

            if (mRect.h != newHeight)
            {
                SDL_Rect newRect = mRect;
                newRect.h = newHeight;
                SetRect(newRect);
            }
        }
    }

    void MdiWindow::SetFullScreen(bool fullScreen)
    {
        mFullScreen = fullScreen;
    }

    bool MdiWindow::IsFullScreen() const
    {
        return mFullScreen;
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

    bool MdiWindow::Intersects(const SDL_Point& point) const
    {
        return ContainsPoint(point);
    }

    void MdiWindow::SetOnClose(const std::function<void(MdiWindow&)>& callback)
    {
        mOnClose = callback;
    }

    SDL_Rect MdiWindow::GetTitleBarRect() const
    {
        const SDL_Rect rect = GetScreenRect();
        return SDL_Rect{.x = rect.x,
                        .y = rect.y,
                        .w = rect.w,
                        .h = SDL_min(rect.h, mWindowStyle.titleBarHeight)};
    }

    SDL_Rect MdiWindow::GetCloseButtonRect() const
    {
        const SDL_Rect titleBar = GetTitleBarRect();
        const int margin = (titleBar.h - mWindowStyle.closeButtonSize) / 2;

        return SDL_Rect{.x = titleBar.x + titleBar.w - mWindowStyle.closeButtonSize - margin,
                        .y = titleBar.y + margin,
                        .w = mWindowStyle.closeButtonSize,
                        .h = mWindowStyle.closeButtonSize};
    }

    SDL_Rect MdiWindow::GetClientRect() const
    {
        if (mSkin != nullptr)
        {
            return mSkin->GetInnerRect(this);
        }

        const SDL_Rect rect = GetScreenRect();

        const int inset = mWindowStyle.borderSize;
        const int topInset = mWindowStyle.titleBarHeight;

        return SDL_Rect{.x = rect.x + inset,
                        .y = rect.y + topInset,
                        .w = SDL_max(0, rect.w - (inset * 2)),
                        .h = SDL_max(0, rect.h - topInset - inset)};
    }
} // namespace nuvelocity
