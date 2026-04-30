#ifndef NVE_UI_MDI_WINDOW_H
#define NVE_UI_MDI_WINDOW_H

#include "API.h"
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
            int titleBarHeight = 24;
            int borderSize = 4;
            int closeButtonSize = 16;
            bool tileBackground = false;
        };

        NVE_API MdiWindow();
        NVE_API explicit MdiWindow(std::string title);

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void SetRect(const SDL_Rect& rect) override;
        NVE_API SDL_Point GetContentOrigin() const override;

        NVE_API void SetTitle(const std::string& title);
        NVE_API const std::string& GetTitle() const;

        NVE_API void SetMovable(bool movable);
        NVE_API bool IsMovable() const;

        NVE_API void SetClosable(bool closable);
        NVE_API bool IsClosable() const;

        NVE_API void SetActive(bool active);
        NVE_API bool IsActive() const;

        NVE_API void SetBackgroundTile(StandAloneFrame* frame);
        NVE_API StandAloneFrame* GetBackgroundTile() const;

        NVE_API void SetStyle(const Style& style);
        NVE_API const Style& GetWindowStyle() const;

        NVE_API void AddChild(const std::shared_ptr<Widget>& widget);
        NVE_API const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

        NVE_API void FitToChildren(Game* game);

        NVE_API void Close();
        NVE_API bool ShouldClose() const;

        NVE_API void SetFullScreen(bool fullScreen);
        NVE_API bool IsFullScreen() const;

        NVE_API bool Intersects(const SDL_Point& point) const;

        NVE_API void SetOnClose(const std::function<void(MdiWindow&)>& callback);

        NVE_API SDL_Rect GetTitleBarRect() const;
        NVE_API SDL_Rect GetCloseButtonRect() const;
        NVE_API SDL_Rect GetClientRect() const;

    private:
        std::string mTitle;
        bool mMovable;
        bool mClosable;
        bool mActive;
        bool mDragging;
        bool mShouldClose;
        bool mAutoResize = false;
        bool mFullScreen = false;

        SDL_Point mDragGrabOffset;
        StandAloneFrame* mBackgroundTile = nullptr;
        Style mWindowStyle;

        std::vector<std::shared_ptr<Widget>> mChildren;
        std::function<void(MdiWindow&)> mOnClose;
    };
} // namespace nuvelocity

#endif // NVE_UI_MDI_WINDOW_H
