#ifndef NVE_UI_WINDOW_H
#define NVE_UI_WINDOW_H

#include "Widget.h"

#include <Image.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nuvelocity
{
    class Window : public Widget
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

        Window();
        explicit Window(std::string title);

        void Update(Game* aGame) override;
        void Draw(Game* game) override;

        void SetRect(const SDL_FRect& rect);

        void SetTitle(const std::string& title);
        const std::string& GetTitle() const;

        void SetMovable(bool movable);
        bool IsMovable() const;

        void SetClosable(bool closable);
        bool IsClosable() const;

        void SetActive(bool active);
        bool IsActive() const;

        void SetBackgroundTile(const Image& image);
        const Image& GetBackgroundTile() const;

        void SetStyle(const Style& style);
        const Style& GetWindowStyle() const;

        void AddChild(const std::shared_ptr<Widget>& widget);
        const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

        void Close();
        bool ShouldClose() const;

        bool Intersects(const SDL_FPoint& point) const;

        void SetOnClose(const std::function<void(Window&)>& callback);

    private:
        SDL_FRect GetTitleBarRect() const;
        SDL_FRect GetCloseButtonRect() const;
        SDL_FRect GetClientRect() const;

        std::string mTitle;
        bool mMovable;
        bool mClosable;
        bool mActive;
        bool mDragging;
        bool mShouldClose;

        SDL_FPoint mDragGrabOffset;
        Image mBackgroundTile;
        Style mWindowStyle;

        std::vector<std::shared_ptr<Widget>> mChildren;
        std::function<void(Window&)> mOnClose;
    };
} // namespace nuvelocity

#endif // NVE_UI_WINDOW_H
