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

    void Label::Update(Game* aGame)
    {
        (void)aGame;
    }

    void Label::Draw(Game* aGame)
    {
        if (aGame == nullptr || aGame->mFont == nullptr || aGame->mSpriteBatch == nullptr)
        {
            return;
        }

        SDL_Rect rect = GetRect();
        aGame->mFont->DrawStringWithFontAt(
            mFont, aGame->mSpriteBatch, mText, rect.x, rect.y, Colors::White, 13);
    }
} // namespace nuvelocity
