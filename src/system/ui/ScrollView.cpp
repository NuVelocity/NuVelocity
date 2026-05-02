#include "ScrollView.h"
#include "WidgetUtils.h"
#include "skin/JWindowSkin.h"
#include <system/Game.h>
#include <system/InputManager.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    ScrollView::ScrollView()
    {
        mVisible = true;
        mEnabled = true;
    }

    void ScrollView::Update(Game* game)
    {
        if (!mVisible || !mEnabled || game == nullptr || game->mInput == nullptr)
        {
            return;
        }

        InputManager& input = *game->mInput;
        const SDL_Point mouse = input.GetMousePosition();

        mUpPressed = false;
        mDownPressed = false;

        const SDL_Rect rect = GetScreenRect();
        const int visibleAreaHeight = rect.h;
        const int totalHeight = GetTotalContentHeight();
        const bool showScrollbar = totalHeight > visibleAreaHeight;
        const int sbX = rect.x + rect.w - (showScrollbar ? 16 : 0);

        if (!showScrollbar && mDraggingThumb)
        {
            mDraggingThumb = false;
        }

        if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
        {
            if (showScrollbar)
            {
                SDL_Rect upBtn = {sbX, rect.y + 1, 15, 15};
                SDL_Rect downBtn = {sbX, rect.y + rect.h - 16, 15, 15};

                if (mouse.x >= upBtn.x && mouse.x < upBtn.x + upBtn.w && mouse.y >= upBtn.y &&
                    mouse.y < upBtn.y + upBtn.h)
                {
                    mUpPressed = true;
                }
                else if (mouse.x >= downBtn.x && mouse.x < downBtn.x + downBtn.w &&
                         mouse.y >= downBtn.y && mouse.y < downBtn.y + downBtn.h)
                {
                    mDownPressed = true;
                }
            }
        }

        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            if (showScrollbar)
            {
                SDL_Rect upBtn = {sbX, rect.y + 1, 15, 15};
                SDL_Rect downBtn = {sbX, rect.y + rect.h - 16, 15, 15};

                if (mouse.x >= upBtn.x && mouse.x < upBtn.x + upBtn.w && mouse.y >= upBtn.y &&
                    mouse.y < upBtn.y + upBtn.h)
                {
                    mScrollOffset -= 20;
                }
                else if (mouse.x >= downBtn.x && mouse.x < downBtn.x + downBtn.w &&
                         mouse.y >= downBtn.y && mouse.y < downBtn.y + downBtn.h)
                {
                    mScrollOffset += 20;
                }
                else if (mouse.x >= sbX && mouse.x < rect.x + rect.w)
                {
                    // Scrollbar track (not buttons)
                    int trackHeight = rect.h - 32;

                    if (totalHeight > visibleAreaHeight)
                    {
                        float ratio = (float)trackHeight / (float)totalHeight;
                        int thumbHeight = SDL_max(10, (int)(visibleAreaHeight * ratio));
                        int thumbY = upBtn.y + 15 + (int)((float)mScrollOffset * ratio);

                        if (mouse.x >= sbX && mouse.x < sbX + 15 && mouse.y >= thumbY &&
                            mouse.y < thumbY + thumbHeight)
                        {
                            mDraggingThumb = true;
                            mDragStartY = mouse.y;
                            mDragStartScroll = mScrollOffset;
                        }
                        else if (mouse.y < thumbY)
                        {
                            mScrollOffset -= visibleAreaHeight;
                        }
                        else if (mouse.y > thumbY + thumbHeight)
                        {
                            mScrollOffset += visibleAreaHeight;
                        }
                    }
                }
            }
        }

        if (mDraggingThumb)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                int deltaY = mouse.y - mDragStartY;
                int trackHeight = rect.h - 32;

                if (totalHeight > visibleAreaHeight)
                {
                    float ratio = (float)trackHeight / (float)totalHeight;
                    mScrollOffset = mDragStartScroll + (int)((float)deltaY / ratio);
                }
            }
            else
            {
                mDraggingThumb = false;
            }
        }

        // Handle Mouse Wheel
        SDL_Point wheel = input.GetWheelDelta();
        if (wheel.y != 0 && ContainsPoint(mouse))
        {
            mScrollOffset -= wheel.y * 20 * 3;
        }

        // Clamp scroll offset
        int maxScroll = SDL_max(0, totalHeight - visibleAreaHeight);
        mScrollOffset = SDL_clamp(mScrollOffset, 0, maxScroll);

        // Update children
        for (auto& child : mChildren)
        {
            if (child != nullptr)
            {
                child->Update(game);
            }
        }
    }

    void ScrollView::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mSpriteBatch == nullptr)
        {
            return;
        }

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawScrollView(game, this);
        }

        const SDL_Rect screenRect = GetScreenRect();
        const int totalHeight = GetTotalContentHeight();
        const bool showScrollbar = totalHeight > screenRect.h;
        const int scrollbarWidth = showScrollbar ? 16 : 0;

        SDL_Rect clipRect = screenRect;
        clipRect.x += 3;
        clipRect.y += 3;
        clipRect.w -= (scrollbarWidth + 6);
        clipRect.h -= 6;

        game->mSpriteBatch->SetClipRect(&clipRect);
        for (auto& child : mChildren)
        {
            if (child != nullptr && child->IsVisible())
            {
                child->Draw(game);
            }
        }
        game->mSpriteBatch->SetClipRect(nullptr);
    }

    void ScrollView::AddChild(const std::shared_ptr<Widget>& child)
    {
        if (child != nullptr)
        {
            child->SetParent(this);
            mChildren.push_back(child);
        }
    }

    int ScrollView::GetTotalContentHeight() const
    {
        int maxBottom = 0;
        for (const auto& child : mChildren)
        {
            if (child != nullptr)
            {
                const SDL_Rect r = child->GetRect();
                maxBottom = SDL_max(maxBottom, r.y + r.h);
            }
        }
        return maxBottom;
    }

    SDL_Point ScrollView::GetContentOrigin() const
    {
        const SDL_Rect rect = GetScreenRect();
        return SDL_Point{.x = rect.x + 3, .y = rect.y + 3 - mScrollOffset};
    }
} // namespace nuvelocity
