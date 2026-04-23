#ifndef NVE_UI_WIDGET_UTILS_H
#define NVE_UI_WIDGET_UTILS_H

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

        static void FillRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color);
        static void DrawRect(SpriteBatch* batch, const SDL_Rect& rect, const SDL_Color& color);
        static void
        DrawTiledFrame(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        static void
        DrawTiledFrameH(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        static void
        DrawTiledFrameV(SpriteBatch* batch, StandAloneFrame* frame, const SDL_Rect& area);
        static void DrawTiledFramePart(SpriteBatch* batch,
                                       StandAloneFrame* frame,
                                       const SDL_Rect& area,
                                       const SDL_Rect& srcRect);
        static void DrawTiledFramePartH(SpriteBatch* batch,
                                        StandAloneFrame* frame,
                                        const SDL_Rect& area,
                                        const SDL_Rect& srcRect);
        static void DrawTiledFramePartV(SpriteBatch* batch,
                                        StandAloneFrame* frame,
                                        const SDL_Rect& area,
                                        const SDL_Rect& srcRect);
        static void DrawBevel(SpriteBatch* batch,
                              const SDL_Rect& rect,
                              const BevelColors& colors,
                              bool sunken,
                              int thickness);
        static void DrawBorder(SpriteBatch* batch,
                               const SDL_Rect& rect,
                               const BorderColors& colors,
                               int thickness);
    };
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_UTILS_H
