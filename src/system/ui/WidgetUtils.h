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

        static void FillRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color);
        static void DrawRect(SpriteBatch* batch, const SDL_FRect& rect, const SDL_Color& color);
        static void
        DrawTiledFrame(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area);
        static void
        DrawTiledFrameH(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area);
        static void
        DrawTiledFrameV(SpriteBatch* batch, StandAloneFrame* frame, const SDL_FRect& area);
        static void DrawTiledFramePart(SpriteBatch* batch,
                                       StandAloneFrame* frame,
                                       const SDL_FRect& area,
                                       const SDL_FRect& srcRect);
        static void DrawTiledFramePartH(SpriteBatch* batch,
                                        StandAloneFrame* frame,
                                        const SDL_FRect& area,
                                        const SDL_FRect& srcRect);
        static void DrawTiledFramePartV(SpriteBatch* batch,
                                        StandAloneFrame* frame,
                                        const SDL_FRect& area,
                                        const SDL_FRect& srcRect);
        static void DrawBevel(SpriteBatch* batch,
                              const SDL_FRect& rect,
                              const BevelColors& colors,
                              bool sunken,
                              float thickness);
    };
} // namespace nuvelocity

#endif // NVE_UI_WIDGET_UTILS_H
