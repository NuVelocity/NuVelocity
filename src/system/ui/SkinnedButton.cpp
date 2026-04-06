#include "SkinnedButton.h"

#include <system/FontManager.h>
#include <system/Game.h>

namespace nuvelocity
{
    SkinnedButton::SkinnedButton() = default;

    void SkinnedButton::Draw(Game* game, const SDL_FPoint& parentOffset)
    {
        if (!mVisible || game == nullptr || game->mFont == nullptr || game->mSpriteBatch == nullptr)
        {
            return;
        }

        Sequence* sequence = nullptr;
        if (!mEnabled)
        {
            sequence = mSkin.disabled;
        }
        else if (mPressed)
        {
            sequence = mSkin.pressed;
        }
        else if (mHovered)
        {
            sequence = mSkin.hover;
        }
        else
        {
            sequence = mSkin.normal;
        }

        if (sequence != nullptr)
        {
            const SDL_FRect rect = GetScreenRect(parentOffset);
            SDL_Surface* surface = sequence->GetSurface(0);
            if (surface != nullptr)
            {
                game->mSpriteBatch->Draw(surface, &rect);
            }

            SDL_FRect textRect{.x = rect.x + 4.0F,
                               .y = rect.y + 2.0F,
                               .w = SDL_max(0.0F, rect.w - 8.0F),
                               .h = SDL_max(0.0F, rect.h - 4.0F)};
            game->mFont->DrawString(game->mSpriteBatch,
                                    mDisplayCaption,
                                    textRect,
                                    mButtonStyle.textColor,
                                    mButtonStyle.fontPointSize,
                                    TextAlignment::Center,
                                    true,
                                    mMnemonicIndex);
            return;
        }

        Button::Draw(game, parentOffset);
    }

    void SkinnedButton::SetSkin(const Skin& skin)
    {
        mSkin = skin;
    }

    const SkinnedButton::Skin& SkinnedButton::GetSkin() const
    {
        return mSkin;
    }
} // namespace nuvelocity
