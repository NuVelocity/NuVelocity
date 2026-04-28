#include "JListBox.h"
#include "WidgetUtils.h"
#include "skin/JWindowSkin.h"
#include <Colors.h>
#include <system/FontManager.h>
#include <system/Game.h>
#include <system/InputManager.h>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    JListBox::JListBox()
    {
        mVisible = true;
        mEnabled = true;
    }

    void JListBox::AddColumn(const std::string& title, int width)
    {
        mColumns.push_back({title, width});
    }

    void JListBox::AddRow(const std::vector<std::string>& cells)
    {
        mRows.push_back(cells);
    }

    void JListBox::Clear()
    {
        mRows.clear();
        mSelectedIndex = -1;
        mScrollOffset = 0;
    }

    int JListBox::GetSelectedIndex() const
    {
        return mSelectedIndex;
    }

    void JListBox::SetSelectedIndex(int index)
    {
        if (index >= -1 && index < static_cast<int>(mRows.size()))
        {
            mSelectedIndex = index;
            if (mOnSelectionChanged)
            {
                mOnSelectionChanged(mSelectedIndex);
            }
        }
    }

    void JListBox::SetColumnWidth(int index, int width)
    {
        if (index >= 0 && index < static_cast<int>(mColumns.size()))
        {
            mColumns[index].width = SDL_max(5, width);
        }
    }

    void JListBox::Update(Game* game)
    {
        if (!mVisible || !mEnabled || game == nullptr || game->mInput == nullptr)
        {
            return;
        }

        InputManager& input = *game->mInput;
        const SDL_Point mouse = input.GetMousePosition();

        mUpPressed = false;
        mDownPressed = false;

        if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
        {
            const SDL_Rect rect = GetScreenRect();
            int sbX = rect.x + rect.w - 16;
            SDL_Rect upBtn = {sbX, rect.y + mHeaderHeight + 1, 15, 15};
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

        // Handle Column Resizing
        if (mResizingColumn != -1)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                int deltaX = mouse.x - mResizeStartX;
                SetColumnWidth(mResizingColumn, mResizeStartWidth + deltaX);
            }
            else
            {
                mResizingColumn = -1;
            }
        }

        if (ContainsPoint(mouse) && input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            const SDL_Rect rect = GetScreenRect();

            // Check header for resizing
            if (mouse.y < rect.y + mHeaderHeight)
            {
                int curX = rect.x + 5; // leftMargin matches theme
                for (int i = 0; i < static_cast<int>(mColumns.size()); ++i)
                {
                    curX += mColumns[i].width;
                    if (SDL_abs(mouse.x - curX) < 4)
                    {
                        mResizingColumn = i;
                        mResizeStartX = mouse.x;
                        mResizeStartWidth = mColumns[i].width;
                        return; // Start resizing, don't handle clicks
                    }
                }
            }

            // Check scroll buttons
            int sbX = rect.x + rect.w - 16;
            SDL_Rect upBtn = {sbX, rect.y + mHeaderHeight + 1, 15, 15};
            SDL_Rect downBtn = {sbX, rect.y + rect.h - 16, 15, 15};

            if (mouse.x >= upBtn.x && mouse.x < upBtn.x + upBtn.w && mouse.y >= upBtn.y &&
                mouse.y < upBtn.y + upBtn.h)
            {
                mScrollOffset -= mRowHeight;
            }
            else if (mouse.x >= downBtn.x && mouse.x < downBtn.x + downBtn.w &&
                     mouse.y >= downBtn.y && mouse.y < downBtn.y + downBtn.h)
            {
                mScrollOffset += mRowHeight;
            }
            else if (mouse.x >= sbX && mouse.x < rect.x + rect.w)
            {
                // Scrollbar track (not buttons)
                // Proportional scroll logic
                int maxVisibleRows = (rect.h - mHeaderHeight) / mRowHeight;
                int totalHeight = (int)mRows.size() * mRowHeight;
                int visibleAreaHeight = rect.h - mHeaderHeight - 32;

                if (totalHeight > visibleAreaHeight + 32)
                {
                    float ratio = (float)visibleAreaHeight / (float)totalHeight;
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
                        mScrollOffset -= maxVisibleRows * mRowHeight;
                    }
                    else if (mouse.y > thumbY + thumbHeight)
                    {
                        mScrollOffset += maxVisibleRows * mRowHeight;
                    }
                }
            }
            else
            {
                int localY = mouse.y - rect.y - mHeaderHeight;
                if (localY >= 0 && mouse.x < sbX)
                {
                    int index = (localY + mScrollOffset) / mRowHeight;
                    if (index >= 0 && index < static_cast<int>(mRows.size()))
                    {
                        SetSelectedIndex(index);
                    }
                }
            }
        }

        if (mDraggingThumb)
        {
            if (input.IsMouseButtonDown(SDL_BUTTON_LEFT))
            {
                int deltaY = mouse.y - mDragStartY;

                int totalHeight = (int)mRows.size() * mRowHeight;
                int visibleAreaHeight = GetRect().h - mHeaderHeight - 32;

                if (totalHeight > visibleAreaHeight + 32)
                {
                    float ratio = (float)visibleAreaHeight / (float)totalHeight;
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
            mScrollOffset -= wheel.y * mRowHeight * 3;
        }

        // Handle Keyboard Navigation
        int prevSelected = mSelectedIndex;
        int maxVisibleRows = (mRect.h - mHeaderHeight) / mRowHeight;

        auto handleNav = [&](SDL_Scancode code)
        {
            if (input.IsKeyPressed(code))
            {
                mLastNavTick = SDL_GetTicks();
                mLastNavCode = code;
                return true;
            }
            if (input.IsKeyDown(code) && mLastNavCode == code)
            {
                uint64_t now = SDL_GetTicks();
                if (now - mLastNavTick > 500) // Initial delay
                {
                    if (now - mLastNavTick > 550) // Repeat rate (~50ms)
                    {
                        mLastNavTick = now - 500; // Reset to just after initial delay
                        return true;
                    }
                }
            }
            return false;
        };

        if (handleNav(SDL_SCANCODE_DOWN))
        {
            SetSelectedIndex(SDL_min(static_cast<int>(mRows.size()) - 1, mSelectedIndex + 1));
        }
        else if (handleNav(SDL_SCANCODE_UP))
        {
            SetSelectedIndex(SDL_max(0, mSelectedIndex - 1));
        }
        else if (handleNav(SDL_SCANCODE_PAGEDOWN))
        {
            SetSelectedIndex(
                SDL_min(static_cast<int>(mRows.size()) - 1, mSelectedIndex + maxVisibleRows));
        }
        else if (handleNav(SDL_SCANCODE_PAGEUP))
        {
            SetSelectedIndex(SDL_max(0, mSelectedIndex - maxVisibleRows));
        }
        else if (handleNav(SDL_SCANCODE_HOME))
        {
            SetSelectedIndex(0);
        }
        else if (handleNav(SDL_SCANCODE_END))
        {
            SetSelectedIndex(static_cast<int>(mRows.size()) - 1);
        }

        if (!input.IsKeyDown(mLastNavCode))
        {
            mLastNavCode = SDL_SCANCODE_UNKNOWN;
        }

        // Auto-scroll to selection
        if (mSelectedIndex != prevSelected && mSelectedIndex != -1)
        {
            int selectionY = mSelectedIndex * mRowHeight;
            if (selectionY < mScrollOffset)
            {
                mScrollOffset = selectionY;
            }
            else if (selectionY + mRowHeight > mScrollOffset + (mRect.h - mHeaderHeight))
            {
                mScrollOffset = selectionY + mRowHeight - (mRect.h - mHeaderHeight);
            }
        }

        // Clamp scroll offset
        int maxScroll = SDL_max(0, (int)mRows.size() * mRowHeight - (mRect.h - mHeaderHeight));
        mScrollOffset = SDL_clamp(mScrollOffset, 0, maxScroll);
    }

    void JListBox::Draw(Game* game)
    {
        if (!mVisible || game == nullptr || game->mMdi == nullptr)
        {
            return;
        }

        JWindowSkin* skin = GetSkin(game);
        if (skin != nullptr)
        {
            skin->DrawListBox(game, this);
        }
    }

    void JListBox::SetOnSelectionChanged(std::function<void(int)> callback)
    {
        mOnSelectionChanged = callback;
    }
} // namespace nuvelocity
