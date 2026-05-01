#include "ToggleButtonGroup.h"

namespace nuvelocity
{
    void ToggleButtonGroup::AddButton(const std::shared_ptr<Button>& button, int id)
    {
        if (button == nullptr)
        {
            return;
        }

        mButtons.push_back({button, id});
        button->SetOnClick([this, id](Game* game) { OnButtonClicked(game, id); });

        // Update visual state if this button happens to be the selected one
        button->SetSelected(mSelectedId == id);
    }

    void ToggleButtonGroup::SetSelectedIndex(int id)
    {
        if (mSelectedId == id)
        {
            return;
        }

        mSelectedId = id;
        for (auto& entry : mButtons)
        {
            entry.button->SetSelected(entry.id == mSelectedId);
        }

        if (mOnSelectionChanged)
        {
            mOnSelectionChanged(mSelectedId);
        }
    }

    int ToggleButtonGroup::GetSelectedIndex() const
    {
        return mSelectedId;
    }

    void ToggleButtonGroup::SetOnSelectionChanged(std::function<void(int)> callback)
    {
        mOnSelectionChanged = callback;
    }

    void ToggleButtonGroup::OnButtonClicked(Game* game, int id)
    {
        (void)game;
        SetSelectedIndex(id);
    }
} // namespace nuvelocity
