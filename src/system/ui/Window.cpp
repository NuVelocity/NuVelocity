#include "Window.h"

#include "WidgetUtils.h"

#include <system/FontManager.h>
#include <system/Game.h>
#include <system/InputManager.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    Window::Window()
            : Window("")
    {
    }

    Window::Window(std::string title)
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

    void Window::Update(InputManager& input, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || !mEnabled)
        {
            return;
        }

        const SDL_FPoint mouse = input.GetMousePosition();
        const SDL_FRect titleBar = GetTitleBarRect(parentOffset);
        const SDL_FRect closeButton = GetCloseButtonRect(parentOffset);

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
            const SDL_FRect screenRect = GetScreenRect(parentOffset);
            mDragGrabOffset = SDL_FPoint{.x = mouse.x - screenRect.x, .y = mouse.y - screenRect.y};
        }

        if (mDragging)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                mRect.x = mouse.x - mDragGrabOffset.x - parentOffset.x;
                mRect.y = mouse.y - mDragGrabOffset.y - parentOffset.y;
            }
            else
            {
                mDragging = false;
            }
        }

        const SDL_FRect clientRect = GetClientRect(parentOffset);
        const SDL_FPoint clientOffset{.x = clientRect.x, .y = clientRect.y};
        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Update(input, clientOffset);
            }
        }
    }

    void Window::Draw(Game* game, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || game == nullptr || game->mRenderer == nullptr || game->mFont == nullptr)
        {
            return;
        }

        const SDL_FRect windowRect = GetScreenRect(parentOffset);
        const SDL_FRect titleRect = GetTitleBarRect(parentOffset);
        const SDL_FRect clientRect = GetClientRect(parentOffset);

        FillRect(game->mRenderer, windowRect, mStyle.backgroundColor);
        DrawBevel(game->mRenderer,
                  windowRect,
                  BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                  false,
                  mWindowStyle.borderSize);

        const SDL_Color titleColor =
            mActive ? mWindowStyle.titleBarColor : mWindowStyle.titleBarInactiveColor;
        FillRect(game->mRenderer, titleRect, titleColor);

        SDL_FRect titleTextRect{.x = titleRect.x + 6.0F,
                                .y = titleRect.y,
                                .w = SDL_max(0.0F, titleRect.w - 24.0F),
                                .h = titleRect.h};
        game->mFont->DrawString(game->mRenderer,
                                mTitle,
                                titleTextRect,
                                mWindowStyle.titleTextColor,
                                mWindowStyle.titleFontPointSize,
                                TextAlignment::Left,
                                true);

        if (mWindowStyle.tileBackground && mBackgroundTile.IsValid() &&
            game->mSpriteBatch != nullptr)
        {
            DrawTiledImage(game->mSpriteBatch, mBackgroundTile, clientRect);
        }
        else
        {
            FillRect(game->mRenderer, clientRect, mWindowStyle.clientColor);
        }

        if (mClosable)
        {
            SDL_FRect closeRect = GetCloseButtonRect(parentOffset);
            FillRect(game->mRenderer, closeRect, mWindowStyle.closeButtonColor);
            DrawBevel(game->mRenderer,
                      closeRect,
                      BevelColors{.light = mStyle.borderLightColor, .dark = mStyle.borderDarkColor},
                      false,
                      1.0F);
            game->mFont->DrawString(game->mRenderer,
                                    "X",
                                    closeRect,
                                    mWindowStyle.titleTextColor,
                                    mWindowStyle.titleFontPointSize,
                                    TextAlignment::Center,
                                    true);
        }

        const SDL_FPoint clientOffset{.x = clientRect.x, .y = clientRect.y};
        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Draw(game, clientOffset);
            }
        }
    }

    void Window::SetTitle(const std::string& title)
    {
        mTitle = title;
    }

    const std::string& Window::GetTitle() const
    {
        return mTitle;
    }

    void Window::SetMovable(bool movable)
    {
        mMovable = movable;
    }

    bool Window::IsMovable() const
    {
        return mMovable;
    }

    void Window::SetClosable(bool closable)
    {
        mClosable = closable;
    }

    bool Window::IsClosable() const
    {
        return mClosable;
    }

    void Window::SetActive(bool active)
    {
        mActive = active;
    }

    bool Window::IsActive() const
    {
        return mActive;
    }

    void Window::SetBackgroundTile(const Image& image)
    {
        mBackgroundTile = image;
    }

    const Image& Window::GetBackgroundTile() const
    {
        return mBackgroundTile;
    }

    void Window::SetStyle(const Style& style)
    {
        mWindowStyle = style;
        Widget::SetStyle(style.baseStyle);
    }

    const Window::Style& Window::GetWindowStyle() const
    {
        return mWindowStyle;
    }

    void Window::AddChild(const std::shared_ptr<Widget>& widget)
    {
        if (widget != nullptr)
        {
            mChildren.push_back(widget);
        }
    }

    const std::vector<std::shared_ptr<Widget>>& Window::GetChildren() const
    {
        return mChildren;
    }

    void Window::Close()
    {
        mShouldClose = true;
        if (mOnClose)
        {
            mOnClose(*this);
        }
    }

    bool Window::ShouldClose() const
    {
        return mShouldClose;
    }

    bool Window::Intersects(const SDL_FPoint& point) const
    {
        return point.x >= mRect.x && point.y >= mRect.y && point.x <= mRect.x + mRect.w &&
               point.y <= mRect.y + mRect.h;
    }

    void Window::SetOnClose(const std::function<void(Window&)>& callback)
    {
        mOnClose = callback;
    }

    SDL_FRect Window::GetTitleBarRect(const SDL_FPoint& parentOffset) const
    {
        const SDL_FRect rect = GetScreenRect(parentOffset);
        return SDL_FRect{.x = rect.x,
                         .y = rect.y,
                         .w = rect.w,
                         .h = SDL_min(rect.h, mWindowStyle.titleBarHeight)};
    }

    SDL_FRect Window::GetCloseButtonRect(const SDL_FPoint& parentOffset) const
    {
        const SDL_FRect titleBar = GetTitleBarRect(parentOffset);
        const float margin = (titleBar.h - mWindowStyle.closeButtonSize) * 0.5F;

        return SDL_FRect{.x = titleBar.x + titleBar.w - mWindowStyle.closeButtonSize - margin,
                         .y = titleBar.y + margin,
                         .w = mWindowStyle.closeButtonSize,
                         .h = mWindowStyle.closeButtonSize};
    }

    SDL_FRect Window::GetClientRect(const SDL_FPoint& parentOffset) const
    {
        const SDL_FRect rect = GetScreenRect(parentOffset);

        const float inset = mWindowStyle.borderSize;
        const float topInset = mWindowStyle.titleBarHeight;

        return SDL_FRect{.x = rect.x + inset,
                         .y = rect.y + topInset,
                         .w = SDL_max(0.0F, rect.w - (inset * 2.0F)),
                         .h = SDL_max(0.0F, rect.h - topInset - inset)};
    }
} // namespace nuvelocity
