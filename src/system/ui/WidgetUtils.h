#ifndef NVE_UI_WIDGET_UTILS_H
#define NVE_UI_WIDGET_UTILS_H

#include "API.h"
#include <SDL3/SDL.h>

namespace nuvelocity
{
    class Game;
    class StandAloneFrame;
    class SpriteBatch;

    class WidgetUtils
    {
    public:
        struct BevelColors
        {
            SDL_Color light = SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255};
            SDL_Color dark = SDL_Color{.r = 96, .g = 96, .b = 96, .a = 255};
        };

        struct BorderColors
        {
            SDL_Color topLeftOuter = SDL_Color{.r = 128, .g = 128, .b = 200, .a = 60};
            SDL_Color topLeftInner = SDL_Color{.r = 128, .g = 128, .b = 200, .a = 60};
            SDL_Color bottomRightInner = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 128};
            SDL_Color bottomRightOuter = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 128};
        };

        enum BorderSide
        {
            Side_None = 0,
            Side_Left = 1 << 0,
            Side_Top = 1 << 1,
            Side_Right = 1 << 2,
            Side_Bottom = 1 << 3,
            Side_All = Side_Left | Side_Top | Side_Right | Side_Bottom
        };

        NVE_API static void
        FillRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color);
        NVE_API static void
        DrawRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color);
        NVE_API static void
        DrawTiledFrame(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        NVE_API static void
        DrawTiledFrameH(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        NVE_API static void
        DrawTiledFrameV(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        NVE_API static void DrawTiledFramePart(SpriteBatch* batch,
                                               StandAloneFrame* frame,
                                               const SDL_Rect& area,
                                               const SDL_Rect& srcRect);
        NVE_API static void DrawTiledFramePartH(SpriteBatch* batch,
                                                StandAloneFrame* frame,
                                                const SDL_Rect& area,
                                                const SDL_Rect& srcRect);
        NVE_API static void DrawTiledFramePartV(SpriteBatch* batch,
                                                StandAloneFrame* frame,
                                                const SDL_Rect& area,
                                                const SDL_Rect& srcRect);
        NVE_API static void DrawBevel(SpriteBatch* batch,
                                      const SDL_Rect& rect,
                                      const BorderColors& colors,
                                      int thickness,
                                      int sides = Side_All);
        NVE_API static void DrawBevel(SpriteBatch* batch,
                                      const SDL_Rect& rect,
                                      const BevelColors& colors,
                                      bool sunken,
                                      int thickness,
                                      int sides = Side_All);
        NVE_API static void DrawBorder(SpriteBatch* batch,
                                       const SDL_Rect& rect,
                                       const BorderColors& colors,
                                       int thickness,
                                       int sides = Side_All);
    };
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_UTILS_H
