#ifndef NVE_UI_FOCUS_CONTAINER_H
#define NVE_UI_FOCUS_CONTAINER_H

#include "API.h"
#include "Widget.h"
#include <cstddef>

namespace nuvelocity
{
    class InputManager;
    class Button;
    class Game;

    class FocusContainer : public Widget
    {
    public:
        NVE_API FocusContainer(Button* items[], std::size_t itemCount);

        NVE_API void UpdateFocusNavigation(InputManager* input);
        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;
        NVE_API bool HasFocus() const;
        NVE_API void ClearFocus();
        NVE_API std::size_t GetFocusedIndex() const;
        NVE_API void SetFocusFromMouseClickCheck(bool clickedOnItem);
        NVE_API void SetFocused(std::size_t index, bool focused);

        NVE_API Button& GetItem(std::size_t index);
        NVE_API const Button& GetItem(std::size_t index) const;
        NVE_API std::size_t GetItemCount() const;

    private:
        Button** mItems;
        std::size_t mItemCount;
        std::size_t mFocusedIndex;
        bool mHasFocus;
    };
} // namespace nuvelocity

#endif // NVE_UI_FOCUS_CONTAINER_H
