#ifndef NVE_JWINDOW_SKIN_H
#define NVE_JWINDOW_SKIN_H

#include "model/Object.h"

#include <string>

namespace nuvelocity
{
    class AssetManager;
    class Game;
    class Button;
    class JListBox;
    class JTabControl;
    class MdiWindow;
    class ScrollView;
    class TextBox;
    class SpriteBatch;
    class StandAloneFrame;

    class JWindowSkin
    {
    public:
        virtual ~JWindowSkin() = default;
        virtual void Load(AssetManager* assets) {}

        virtual void DrawButton(Game* game, Button* button) = 0;
        virtual void DrawListBox(Game* game, JListBox* listBox) = 0;
        virtual void DrawMdiWindow(Game* game, MdiWindow* window) = 0;
        virtual void DrawTextBox(Game* game, TextBox* textBox) = 0;
        virtual void DrawTabControl(Game* game, JTabControl* tabControl) = 0;
        virtual void DrawScrollView(Game* game, ScrollView* scrollView) = 0;

        // Returns pixel width of text rendered at the given point size using this skin's font.
        virtual int MeasureTextWidth(Game* game, const std::string& text, int pointSize) const = 0;

        virtual SDL_Rect GetInnerRect(const MdiWindow* window) const = 0;
        virtual SDL_Rect GetCloseButtonRect(const MdiWindow* window) const = 0;
        virtual int GetTabPointSize() const = 0;
    };
} // namespace nuvelocity

#endif // NVE_JWINDOW_SKIN_H
