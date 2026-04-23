#ifndef NVE_JWINDOW_SKIN_H
#define NVE_JWINDOW_SKIN_H

#include "model/Object.h"

namespace nuvelocity
{
    class AssetManager;
    class Game;
    class Button;
    class MdiWindow;
    class TextBox;
    class SpriteBatch;
    class StandAloneFrame;

    class JWindowSkin
    {
    public:
        virtual ~JWindowSkin() = default;
        virtual void Load(AssetManager* assets) {}

        virtual void DrawButton(Game* game, Button* button) = 0;
        virtual void DrawMdiWindow(Game* game, MdiWindow* window) = 0;
        virtual void DrawTextBox(Game* game, TextBox* textBox) = 0;

        virtual SDL_Rect GetInnerRect(const MdiWindow* window) const = 0;
        virtual SDL_Rect GetCloseButtonRect(const MdiWindow* window) const = 0;
    };
} // namespace nuvelocity

#endif // NVE_JWINDOW_SKIN_H
