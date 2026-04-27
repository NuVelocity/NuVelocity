#include "SpriteBatch.h"

namespace nuvelocity
{
    void SpriteBatch::Draw(StandAloneFrame* frame, int x, int y, SDL_Color color, bool useHotSpot)
    {
        if (frame == nullptr)
        {
            return;
        }

        useHotSpot = useHotSpot && frame->GetCenterHotSpot();

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        SDL_Rect destRect{.x = x + (useHotSpot ? frame->GetHotSpot().x + (surface->w / 2) : 0),
                          .y = y + (useHotSpot ? frame->GetHotSpot().y + (surface->h / 2) : 0),
                          .w = surface->w,
                          .h = surface->h};

        Draw(surface, &destRect, nullptr, color);
    }

    void SpriteBatch::Draw(
        Sequence* sequence, std::size_t frameIndex, int x, int y, SDL_Color color, bool useHotSpot)
    {
        if (sequence == nullptr || frameIndex >= sequence->GetFrameCount())
        {
            return;
        }

        useHotSpot = useHotSpot && sequence->GetCenterHotSpot();

        Frame* frame = sequence->GetFrame(frameIndex);
        if (frame == nullptr)
        {
            return;
        }

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        SDL_Rect destRect{.x = x + (useHotSpot ? frame->GetHotSpot().x : 0),
                          .y = y + (useHotSpot ? frame->GetHotSpot().y : 0),
                          .w = surface->w,
                          .h = surface->h};
        Draw(surface, &destRect, nullptr, color);
    }

    void SpriteBatch::DrawCentered(StandAloneFrame* frame)
    {
        if (frame == nullptr)
        {
            return;
        }

        SDL_Surface* surface = frame->GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        DrawCentered(surface);
    }

    void SpriteBatch::DrawCentered(Sequence* sequence, std::size_t frameIndex)
    {
        if (sequence == nullptr || frameIndex >= sequence->GetFrameCount())
        {
            return;
        }

        SDL_Surface* surface = sequence->GetSurface(frameIndex);
        if (surface == nullptr)
        {
            return;
        }

        DrawCentered(surface);
    }

    void SpriteBatch::OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness)
    {
        DrawLine(rect->x, rect->y, rect->x + rect->w, rect->y, color, thickness);
        DrawLine(
            rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h, color, thickness);
        DrawLine(
            rect->x + rect->w, rect->y + rect->h, rect->x, rect->y + rect->h, color, thickness);
        DrawLine(rect->x, rect->y + rect->h, rect->x, rect->y, color, thickness);
    }
} // namespace nuvelocity
