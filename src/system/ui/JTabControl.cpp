#include "JTabControl.h"
#include "skin/JWindowSkin.h"
#include "SpriteBatch.h"
#include <system/Game.h>
#include <system/InputManager.h>

namespace nuvelocity
{
    JTabControl::JTabControl() {}

    void JTabControl::AddTab(const std::string& caption)
    {
        mTabs.push_back(caption);
        if (mSelectedIndex < 0)
        {
            mSelectedIndex = 0;
        }
    }

    void JTabControl::ClearTabs()
    {
        mTabs.clear();
        mSelectedIndex = -1;
    }

    int JTabControl::GetSelectedIndex() const
    {
        return mSelectedIndex;
    }

    void JTabControl::SetSelectedIndex(int index)
    {
        if (mTabs.empty())
        {
            mSelectedIndex = -1;
            return;
        }

        int newIndex = index;
        if (newIndex < 0)
        {
            newIndex = static_cast<int>(mTabs.size()) - 1;
        }
        else if (newIndex >= static_cast<int>(mTabs.size()))
        {
            newIndex = 0;
        }

        if (mSelectedIndex != newIndex)
        {
            mSelectedIndex = newIndex;
            if (mOnTabChanged)
            {
                mOnTabChanged(mSelectedIndex);
            }
        }
    }

    const std::vector<std::string>& JTabControl::GetTabs() const
    {
        return mTabs;
    }

    void JTabControl::SetOnTabChanged(std::function<void(int)> callback)
    {
        mOnTabChanged = callback;
    }

    void JTabControl::Update(Game* game)
    {
        Widget::Update(game);

        if (game == nullptr || game->mInput == nullptr || mTabs.empty())
        {
            return;
        }

        InputManager& input = *game->mInput;

        // Ctrl + Tab / Ctrl + Shift + Tab
        const bool ctrlDown =
            input.IsKeyDown(SDL_SCANCODE_LCTRL) || input.IsKeyDown(SDL_SCANCODE_RCTRL);
        if (ctrlDown && input.IsKeyPressed(SDL_SCANCODE_TAB))
        {
            const bool shiftDown =
                input.IsKeyDown(SDL_SCANCODE_LSHIFT) || input.IsKeyDown(SDL_SCANCODE_RSHIFT);
            if (shiftDown)
            {
                SetSelectedIndex(mSelectedIndex - 1);
            }
            else
            {
                SetSelectedIndex(mSelectedIndex + 1);
            }
        }

        // Mouse click on tabs
        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            SDL_Point mouse = input.GetMousePosition();
            UpdateTabRects(game);
            for (const auto& tab : mTabRects)
            {
                if (SDL_PointInRect(&mouse, &tab.rect))
                {
                    SetSelectedIndex(tab.index);
                    break;
                }
            }
        }

        // Update children (container behavior)
        for (const std::shared_ptr<Widget>& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Update(game);
            }
        }
    }

    void JTabControl::Draw(Game* game)
    {
        if (!mVisible || game == nullptr)
        {
            return;
        }

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawTabControl(game, this);
        }

        // Clip children to the tab content area so they don't draw over the
        // tab header.
#if !DEBUG
        SDL_Rect clipRect{.x = 0, .y = 0, .w = 0, .h = 0};
        const SDL_Rect s = GetScreenRect();
        clipRect.x = s.x;
        clipRect.y = s.y + mTabHeaderHeight;
        clipRect.w = s.w;
        clipRect.h = SDL_max(0, s.h - mTabHeaderHeight);
        game->mSpriteBatch->SetClipRect(&clipRect);
#endif

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
    }

    void JTabControl::AddChild(const std::shared_ptr<Widget>& widget)
    {
        if (widget != nullptr)
        {
            widget->SetParent(this);
            mChildren.push_back(widget);
        }
    }

    const std::vector<std::shared_ptr<Widget>>& JTabControl::GetChildren() const
    {
        return mChildren;
    }

    SDL_Point JTabControl::GetContentOrigin() const
    {
        const SDL_Rect rect = GetScreenRect();
        return SDL_Point{.x = rect.x, .y = rect.y + mTabHeaderHeight};
    }

    void JTabControl::UpdateTabRects(Game* game)
    {
        mTabRects.clear();
        if (game == nullptr || mTabs.empty())
        {
            return;
        }

        JWindowSkin* skin = GetSkin(game);
        if (skin == nullptr)
        {
            return;
        }

        SDL_Rect screenRect = GetScreenRect();
        int tabHeight = mTabHeaderHeight;

        // First compute base widths from text measurements
        std::vector<int> widths;
        widths.reserve(mTabs.size());
        int totalWidth = 0;
        int pointSize = skin->GetTabPointSize();
        for (int i = 0; i < static_cast<int>(mTabs.size()); ++i)
        {
            int textWidth = skin->MeasureTextWidth(game, mTabs[i], pointSize);
            int tabWidth = textWidth + (mTabPadding * 2);
            widths.push_back(tabWidth);
            totalWidth += tabWidth;
        }

        // Build final rects
        int curX = screenRect.x;
        for (int i = 0; i < static_cast<int>(mTabs.size()); ++i)
        {
            int w = widths[i];
            mTabRects.push_back({{curX, screenRect.y, w, tabHeight}, i});
            curX += w;
        }
    }
} // namespace nuvelocity
