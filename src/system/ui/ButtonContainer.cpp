#include "ButtonContainer.h"
#include "Button.h"
#include "MdiWindow.h"
#include <algorithm>

namespace nuvelocity
{
    ButtonContainer::ButtonContainer()
    {
    }

    void ButtonContainer::AddButton(const std::shared_ptr<Button>& button)
    {
        if (button != nullptr)
        {
            mButtons.push_back(button);
            button->SetParent(this);
        }
    }

    void ButtonContainer::Update(Game* aGame)
    {
        if (!mVisible || !mEnabled)
        {
            return;
        }

        if (mAutoCenter && mParent != nullptr)
        {
            SDL_FRect parentRect;
            if (auto* mdi = dynamic_cast<MdiWindow*>(mParent))
            {
                parentRect = mdi->GetClientRect();
            }
            else
            {
                parentRect = mParent->GetScreenRect();
            }

            const float centerX = parentRect.x + (parentRect.w - mRect.w) * 0.5f;
            if (SDL_fabs(mRect.x - centerX) > 0.5f)
            {
                mRect.x = centerX;
            }
        }

        float currentY = mPadding.top;
        float availableWidth = mRect.w - mPadding.left - mPadding.right;

        for (auto& btn : mButtons)
        {
            SDL_FRect btnRect = btn->GetRect();
            if (btnRect.w > 0 && btnRect.w < availableWidth)
            {
                btnRect.x = mRect.x + mPadding.left + (availableWidth - btnRect.w) * 0.5f;
            }
            else
            {
                btnRect.x = mRect.x + mPadding.left;
                btnRect.w = SDL_max(0.0f, availableWidth);
            }
            btnRect.y = mRect.y + currentY;
            btn->SetRect(btnRect);

            currentY += btnRect.h + mSpacing;
            btn->Update(aGame);
        }

        mRect.h = SDL_max(0.0f, currentY - (mButtons.empty() ? 0 : mSpacing) + mPadding.bottom);
    }

    void ButtonContainer::Draw(Game* aGame)
    {
        if (!mVisible)
        {
            return;
        }

        for (auto& btn : mButtons)
        {
            btn->Draw(aGame);
        }
    }

    void ButtonContainer::SetSpacing(float spacing)
    {
        mSpacing = spacing;
    }

    float ButtonContainer::GetSpacing() const
    {
        return mSpacing;
    }

    void ButtonContainer::SetPadding(float padding)
    {
        mPadding = {padding, padding, padding, padding};
    }

    void ButtonContainer::SetPadding(float horizontal, float vertical)
    {
        mPadding = {vertical, horizontal, vertical, horizontal};
    }

    void ButtonContainer::SetPadding(float top, float right, float bottom, float left)
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
