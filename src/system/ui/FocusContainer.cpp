#include "FocusContainer.h"

#include <system/InputManager.h>
#include <system/ui/Button.h>

namespace nuvelocity
{
    FocusContainer::FocusContainer(Button* items[], std::size_t itemCount)
            : mItems(items)
            , mItemCount(itemCount)
            , mFocusedIndex(0)
            , mHasFocus(false)
    {
    }

    void FocusContainer::UpdateFocusNavigation(InputManager* input)
    {
        if (input == nullptr || mItemCount == 0)
        {
            return;
        }

        if (input->IsKeyPressed(SDL_SCANCODE_TAB))
        {
            if (!mHasFocus)
            {
                const bool shiftDown =
                    input->IsKeyDown(SDL_SCANCODE_LSHIFT) || input->IsKeyDown(SDL_SCANCODE_RSHIFT);
                mFocusedIndex = shiftDown ? (mItemCount - 1) : 0;
                mHasFocus = true;
            }
            else
            {
                const bool shiftDown =
                    input->IsKeyDown(SDL_SCANCODE_LSHIFT) || input->IsKeyDown(SDL_SCANCODE_RSHIFT);
                if (shiftDown)
                {
                    mFocusedIndex = (mFocusedIndex == 0) ? (mItemCount - 1) : (mFocusedIndex - 1);
                }
                else
                {
                    mFocusedIndex = (mFocusedIndex + 1) % mItemCount;
                }
            }
        }
    }

    bool FocusContainer::HasFocus() const
    {
        return mHasFocus;
    }

    void FocusContainer::ClearFocus()
    {
        mHasFocus = false;
    }

    std::size_t FocusContainer::GetFocusedIndex() const
    {
        return mFocusedIndex;
    }

    void FocusContainer::SetFocusFromMouseClickCheck(bool clickedOnItem)
    {
        if (!clickedOnItem)
        {
            mHasFocus = false;
        }
    }

    void FocusContainer::SetFocused(std::size_t index, bool focused)
    {
        if (focused && !mHasFocus)
        {
            mHasFocus = true;
        }
        else if (!focused)
        {
            mHasFocus = false;
        }
        mFocusedIndex = index;
    }

    Button& FocusContainer::GetItem(std::size_t index)
    {
        return *mItems[index];
    }

    const Button& FocusContainer::GetItem(std::size_t index) const
    {
        return *mItems[index];
    }

    std::size_t FocusContainer::GetItemCount() const
    {
        return mItemCount;
    }
} // namespace nuvelocity
