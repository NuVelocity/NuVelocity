#ifndef NVE_JWINDOW_CLASSIC_THEME_H
#define NVE_JWINDOW_CLASSIC_THEME_H

#include "ClassicSkinBorder.h"
#include "JWindowSkin.h"

namespace nuvelocity
{
    class JWindowClassicTheme : public JWindowSkin, public Object<JWindowClassicTheme>
    {
    public:
        void DrawButton(Game* game, Button* button) override;
        void DrawListBox(Game* game, JListBox* listBox) override;
        void DrawMdiWindow(Game* game, MdiWindow* window) override;
        void DrawTextBox(Game* game, TextBox* textBox) override;
        void DrawTabControl(Game* game, JTabControl* tabControl) override;
        void DrawScrollView(Game* game, ScrollView* scrollView) override;

        SDL_Rect GetInnerRect(const MdiWindow* window) const override;
        SDL_Rect GetCloseButtonRect(const MdiWindow* window) const override;
        int MeasureTextWidth(Game* game, const std::string& text, int pointSize) const override;
        int GetTabPointSize() const override
        {
            return 8;
        }

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CJWindowClassicTheme";
        }
    };
} // namespace nuvelocity

#endif // NVE_JWINDOW_CLASSIC_THEME_H
