#ifndef NVE_UI_TOGGLE_BUTTON_GROUP_H
#define NVE_UI_TOGGLE_BUTTON_GROUP_H

#include "API.h"
#include "Button.h"
#include <functional>
#include <memory>
#include <vector>

namespace nuvelocity
{
    class ToggleButtonGroup
    {
    public:
        NVE_API void AddButton(const std::shared_ptr<Button>& button, int id);
        NVE_API void SetSelectedIndex(int id);
        NVE_API int GetSelectedIndex() const;
        NVE_API void SetOnSelectionChanged(std::function<void(int)> callback);

    private:
        struct Entry
        {
            std::shared_ptr<Button> button;
            int id;
        };
        std::vector<Entry> mButtons;
        int mSelectedId = -1;
        std::function<void(int)> mOnSelectionChanged;

        void OnButtonClicked(Game* game, int id);
    };
} // namespace nuvelocity

#endif // NVE_UI_TOGGLE_BUTTON_GROUP_H
