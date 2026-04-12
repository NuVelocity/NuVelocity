#include "MdiWindow.h"

#include "WidgetUtils.h"

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
        const SDL_FRect titleBar = GetTitleBarRect();
        const SDL_FRect closeButton = GetCloseButtonRect();

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
    }

    void MdiWindow::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_FRect windowRect = GetScreenRect();
        const SDL_FRect titleRect = GetTitleBarRect();
        const SDL_FRect clientRect = GetClientRect();

        FillRect(game->mSpriteBatch, windowRect, mStyle.backgroundColor);
        DrawBevel(game->mSpriteBatch,
                  windowRect,
                  BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                  false,
                  mWindowStyle.borderSize);

        const SDL_Color titleColor =
            mActive ? mWindowStyle.titleBarColor : mWindowStyle.titleBarInactiveColor;
        FillRect(game->mSpriteBatch, titleRect, titleColor);

        SDL_FRect titleTextRect{.x = titleRect.x + 6.0F,
                                .y = titleRect.y,
                                .w = SDL_max(0.0F, titleRect.w - 24.0F),
                                .h = titleRect.h};
        titleTextRect.h = titleRect.h;

        game->mFont->DrawString(game->mSpriteBatch,
                                mTitle,
                                titleTextRect,
                                mWindowStyle.titleTextColor,
                                mWindowStyle.titleFontPointSize,
                                TextAlignment::Left,
                                true);

        if (mWindowStyle.tileBackground && mBackgroundTile.IsValid())
        {
            DrawTiledImage(game->mSpriteBatch, mBackgroundTile, clientRect);
        }
        else
        {
            FillRect(game->mSpriteBatch, clientRect, mWindowStyle.clientColor);
        }

        if (mClosable)
        {
            SDL_FRect closeRect = GetCloseButtonRect();
            FillRect(game->mSpriteBatch, closeRect, mWindowStyle.closeButtonColor);
            DrawBevel(game->mSpriteBatch,
                      closeRect,
                      BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                      false,
                      1.0F);
            game->mFont->DrawString(game->mSpriteBatch,
                                    "X",
                                    closeRect,
                                    mWindowStyle.titleTextColor,
                                    mWindowStyle.titleFontPointSize,
                                    TextAlignment::Center,
                                    true);
        }

        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Draw(game);
            }
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

    void MdiWindow::SetBackgroundTile(const Image& image)
    {
        mBackgroundTile = image;
    }

    const Image& MdiWindow::GetBackgroundTile() const
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
        const SDL_FRect rect = GetScreenRect();

        const float inset = mWindowStyle.borderSize;
        const float topInset = mWindowStyle.titleBarHeight;

        return SDL_FRect{.x = rect.x + inset,
                         .y = rect.y + topInset,
                         .w = SDL_max(0.0F, rect.w - (inset * 2.0F)),
                         .h = SDL_max(0.0F, rect.h - topInset - inset)};
    }
} // namespace nuvelocity
