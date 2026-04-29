#include "Label.h"

#include "Colors.h"
#include "Game.h"
#include "system/FontManager.h"
#include <utility>

namespace nuvelocity
{
    Label::Label(std::string text, std::string font)
            : mText(std::move(text))
            , mFont(std::move(font))
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
        game->mFont->DrawStringWithFontAt(mFont,
                                          game->mSpriteBatch,
                                          mText,
                                          rect.x,
                                          rect.y,
                                          Colors::White,
                                          13,
                                          TextAlignment::Left,
                                          nullptr,
                                          -1,
                                          Colors::White);
    }
} // namespace nuvelocity
