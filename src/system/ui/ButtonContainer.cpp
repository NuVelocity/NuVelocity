#include "ButtonContainer.h"
#include "Button.h"
#include "MdiWindow.h"
#include <algorithm>

namespace nuvelocity
{
    ButtonContainer::ButtonContainer() {}

    void ButtonContainer::AddButton(const std::shared_ptr<Button>& button)
    {
        if (button != nullptr)
        {
            mButtons.push_back(button);
            button->SetParent(this);
        }
    }

    void ButtonContainer::Update(Game* game)
    {
        if (!mVisible || !mEnabled)
        {
            return;
        }

        if (mAutoCenter && mParent != nullptr)
        {
            SDL_Rect parentRect;
            if (auto* mdi = dynamic_cast<MdiWindow*>(mParent))
            {
                parentRect = mdi->GetClientRect();
            }
            else
            {
                parentRect = mParent->GetScreenRect();
            }

            const int centerX = parentRect.x + (parentRect.w - mRect.w) / 2;
            if (mRect.x != centerX)
            {
                mRect.x = centerX;
            }
        }

        int currentY = mPadding.top;
        int availableWidth = mRect.w - mPadding.left - mPadding.right;

        for (auto& btn : mButtons)
        {
            SDL_Rect btnRect = btn->GetRect();
            if (btnRect.w > 0 && btnRect.w < availableWidth)
            {
                btnRect.x = mRect.x + mPadding.left + (availableWidth - btnRect.w) / 2;
            }
            else
            {
                btnRect.x = mRect.x + mPadding.left;
                btnRect.w = SDL_max(0, availableWidth);
            }
            btnRect.y = mRect.y + currentY;
            btn->SetRect(btnRect);

            currentY += btnRect.h + mSpacing;
            btn->Update(game);
        }

        mRect.h = SDL_max(0, currentY - (mButtons.empty() ? 0 : mSpacing) + mPadding.bottom);
    }

    void ButtonContainer::Draw(Game* game)
    {
        if (!mVisible)
        {
            return;
        }

        for (auto& btn : mButtons)
        {
            btn->Draw(game);
        }
    }

    void ButtonContainer::SetSpacing(int spacing)
    {
        mSpacing = spacing;
    }

    int ButtonContainer::GetSpacing() const
    {
        return mSpacing;
    }

    void ButtonContainer::SetPadding(int padding)
    {
        mPadding = {padding, padding, padding, padding};
    }

    void ButtonContainer::SetPadding(int horizontal, int vertical)
    {
        mPadding = {vertical, horizontal, vertical, horizontal};
    }

    void ButtonContainer::SetPadding(int top, int right, int bottom, int left)
    {
        mPadding = {top, right, bottom, left};
    }

    const ButtonContainer::Padding& ButtonContainer::GetPadding() const
    {
        return mPadding;
    }

    void ButtonContainer::SetAutoCenter(bool autoCenter)
    {
        mAutoCenter = autoCenter;
    }

    bool ButtonContainer::IsAutoCenter() const
    {
        return mAutoCenter;
    }
} // namespace nuvelocity
