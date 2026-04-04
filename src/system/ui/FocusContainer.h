#ifndef NVE_UI_FOCUS_CONTAINER_H
#define NVE_UI_FOCUS_CONTAINER_H

#include <cstddef>

namespace nuvelocity
{
    class InputManager;
    class Button;

    class FocusContainer
    {
    public:
        FocusContainer(Button* items[], std::size_t itemCount);

        void UpdateFocusNavigation(InputManager* input);
        bool HasFocus() const;
        void ClearFocus();
        std::size_t GetFocusedIndex() const;
        void SetFocusFromMouseClickCheck(bool clickedOnItem);
        void SetFocused(std::size_t index, bool focused);

        Button& GetItem(std::size_t index);
        const Button& GetItem(std::size_t index) const;
        std::size_t GetItemCount() const;

    private:
        Button** mItems;
        std::size_t mItemCount;
        std::size_t mFocusedIndex;
        bool mHasFocus;
    };
} // namespace nuvelocity

#endif // NVE_UI_FOCUS_CONTAINER_H
