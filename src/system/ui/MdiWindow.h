#ifndef NVE_UI_MDI_WINDOW_H
#define NVE_UI_MDI_WINDOW_H

#include "Widget.h"

#include <SDL3/SDL.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nuvelocity
{
    class JWindowSkin;
    class StandAloneFrame;

    class MdiWindow : public Widget
    {
    public:
        struct Style
        {
            WidgetStyle baseStyle = WidgetStyle{};
            SDL_Color titleBarColor = SDL_Color{.r = 0, .g = 0, .b = 128, .a = 255};
            SDL_Color titleBarInactiveColor = SDL_Color{.r = 96, .g = 96, .b = 128, .a = 255};
            SDL_Color titleTextColor = SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255};
            SDL_Color clientColor = SDL_Color{.r = 212, .g = 208, .b = 200, .a = 255};
            SDL_Color closeButtonColor = SDL_Color{.r = 192, .g = 192, .b = 192, .a = 255};
            int titleFontPointSize = 12;
            float titleBarHeight = 24.0F;
            float borderSize = 2.0F;
            float closeButtonSize = 16.0F;
            bool tileBackground = false;
        };

        MdiWindow();
        explicit MdiWindow(std::string title);

        void Update(Game* aGame) override;
        void Draw(Game* game) override;

        void SetRect(const SDL_FRect& rect) override;

        void SetTitle(const std::string& title);
        const std::string& GetTitle() const;

        void SetMovable(bool movable);
        bool IsMovable() const;

        void SetClosable(bool closable);
        bool IsClosable() const;

        void SetActive(bool active);
        bool IsActive() const;

        void SetBackgroundTile(StandAloneFrame* frame);
        StandAloneFrame* GetBackgroundTile() const;

        void SetStyle(const Style& style);
        const Style& GetWindowStyle() const;

        void AddChild(const std::shared_ptr<Widget>& widget);
        const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

        void Close();
        bool ShouldClose() const;

        void SetAutoResize(bool autoResize);
        bool IsAutoResize() const;

        void SetAutoCenter(bool autoCenter);
        bool IsAutoCenter() const;

        bool Intersects(const SDL_FPoint& point) const;

        void SetOnClose(const std::function<void(MdiWindow&)>& callback);

        SDL_FRect GetTitleBarRect() const;
        SDL_FRect GetCloseButtonRect() const;
        SDL_FRect GetClientRect() const;

    private:
        std::string mTitle;
        bool mMovable;
        bool mClosable;
        bool mActive;
        bool mDragging;
        bool mShouldClose;
        bool mAutoResize = false;
        bool mAutoCenter = false;

        SDL_FPoint mDragGrabOffset;
        StandAloneFrame* mBackgroundTile = nullptr;
        Style mWindowStyle;

        std::vector<std::shared_ptr<Widget>> mChildren;
        std::function<void(MdiWindow&)> mOnClose;
    };
} // namespace nuvelocity

#endif // NVE_UI_MDI_WINDOW_H
