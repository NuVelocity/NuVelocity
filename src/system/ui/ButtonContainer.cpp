#include "ButtonContainer.h"
#include "Button.h"
#include "MdiWindow.h"
#include <algorithm>

namespace nuvelocity
{
    ButtonContainer::ButtonContainer() = default;

    void ButtonContainer::AddButton(const std::shared_ptr<Button>& button)
    {
        if (button != nullptr)
        {
            mButtons.push_back(button);
            button->SetParent(this);
            InvalidateLayout();
        }
    }

    void ButtonContainer::Update(Game* game)
    {
        if (!mVisible || !mEnabled)
        {
            return;
        }

        if (mNeedsLayout)
        {
            DoLayout();
            mNeedsLayout = false;
        }

        for (auto& btn : mButtons)
        {
            btn->Update(game);
        }
    }

    void ButtonContainer::SetRect(const SDL_Rect& rect)
    {
        if (mRect.w != rect.w)
        {
            InvalidateLayout();
        }
        Widget::SetRect(rect);
    }

    void ButtonContainer::InvalidateLayout()
    {
        mNeedsLayout = true;
    }

    void ButtonContainer::DoLayout()
    {
        if (mAutoCenter && mParent != nullptr)
        {
            int parentWidth = 0;
            if (auto* mdi = dynamic_cast<MdiWindow*>(mParent))
            {
                parentWidth = mdi->GetClientRect().w;
            }
            else
            {
                parentWidth = mParent->GetRect().w;
            }

            const int centerX = (parentWidth - mRect.w) / 2;
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
                btnRect.x = mPadding.left + ((availableWidth - btnRect.w) / 2);
            }
            else
            {
                btnRect.x = mPadding.left;
                btnRect.w = SDL_max(0, availableWidth);
            }
            btnRect.y = currentY;
            btn->SetRect(btnRect);

            currentY += btnRect.h + mSpacing;
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
        InvalidateLayout();
    }

    int ButtonContainer::GetSpacing() const
    {
        return mSpacing;
    }

    void ButtonContainer::SetPadding(int padding)
    {
        mPadding = {.top = padding, .right = padding, .bottom = padding, .left = padding};
        InvalidateLayout();
    }

    void ButtonContainer::SetPadding(int horizontal, int vertical)
    {
        mPadding = {.top = vertical, .right = horizontal, .bottom = vertical, .left = horizontal};
        InvalidateLayout();
    }

    void ButtonContainer::SetPadding(int top, int right, int bottom, int left)
    {
        mPadding = {.top = top, .right = right, .bottom = bottom, .left = left};
        InvalidateLayout();
    }

    const ButtonContainer::Padding& ButtonContainer::GetPadding() const
    {
        return mPadding;
    }

    void ButtonContainer::SetAutoCenter(bool autoCenter)
    {
        mAutoCenter = autoCenter;
        InvalidateLayout();
    }

    bool ButtonContainer::IsAutoCenter() const
    {
        return mAutoCenter;
    }
} // namespace nuvelocity
