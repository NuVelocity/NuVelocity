#include "Label.h"
#include "Colors.h"
#include "Game.h"
#include "system/FontManager.h"

namespace nuvelocity
{
    Label::Label(const std::string& text, const std::string& font)
            : mText(text)
            , mFont(font)
    {
    }

    void Label::Update(Game* game)
    {
        (void)game;
    }

    void Label::Draw(Game* game)
    {
        if (game == nullptr || game->mFont == nullptr || game->mSpriteBatch == nullptr)
        {
            return;
        }

        SDL_Rect rect = GetRect();
        game->mFont->DrawStringWithFontAt(
            mFont, game->mSpriteBatch, mText, rect.x, rect.y, Colors::White, 13);
    }
} // namespace nuvelocity
