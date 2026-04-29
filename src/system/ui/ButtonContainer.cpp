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

        Widget::Update(game);

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

        int currentY = mStyle.margin.top;
        int availableWidth = mRect.w - mStyle.margin.left - mStyle.margin.right;

        for (auto& btn : mButtons)
        {
            SDL_Rect btnRect = btn->GetRect();
            if (btnRect.w > 0 && btnRect.w < availableWidth)
            {
                btnRect.x = mStyle.margin.left + ((availableWidth - btnRect.w) / 2);
            }
            else
            {
                btnRect.x = mStyle.margin.left;
                btnRect.w = SDL_max(0, availableWidth);
            }
            btnRect.y = currentY;
            btn->SetRect(btnRect);

            currentY += btn->GetActualRect().h + mGap;
        }

        mRect.h = SDL_max(0, currentY - (mButtons.empty() ? 0 : mGap) + mStyle.margin.bottom);

        Widget::DoLayout();
        mNeedsLayout = false;
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

    void ButtonContainer::SetGap(int gap)
    {
        mGap = gap;
        InvalidateLayout();
    }

    int ButtonContainer::GetGap() const
    {
        return mGap;
    }

    void ButtonContainer::SetMargin(int margin)
    {
        mStyle.margin = {.left = margin, .top = margin, .right = margin, .bottom = margin};
        InvalidateLayout();
    }

    void ButtonContainer::SetMargin(int horizontal, int vertical)
    {
        mStyle.margin = {
            .left = horizontal, .top = vertical, .right = horizontal, .bottom = vertical};
        InvalidateLayout();
    }

    void ButtonContainer::SetMargin(int top, int right, int bottom, int left)
    {
        mStyle.margin = {.left = left, .top = top, .right = right, .bottom = bottom};
        InvalidateLayout();
    }

    const Insets& ButtonContainer::GetMargin() const
    {
        return mStyle.margin;
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
