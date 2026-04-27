#ifndef NVE_COLLIDABLE2D_H
#define NVE_COLLIDABLE2D_H

#include "Colors.h"
#include "Frame.h"
#include "Game.h"
#include "GameComponent.h"

#include <SDL3/SDL.h>
#include <cmath>

namespace nuvelocity
{
    class Collidable2D : public GameComponent
    {
    public:
        ~Collidable2D() override = default;

        void SetPosition(const SDL_FPoint& pos)
        {
            mPosition = pos;
        }

        const SDL_FPoint& GetPosition() const
        {
            return mPosition;
        }

        virtual std::vector<SDL_FPoint> GetCollisionPolygon() const = 0;

        virtual void OnHit() {}

        virtual bool IsDestroyed() const
        {
            return false;
        }

        void Draw(Game* game) override
        {
            if (!game->mSpriteBatch->IsDrawBoundsEnabled())
            {
                return;
            }

            const auto& poly = GetCollisionPolygon();
            for (size_t i = 0; i < poly.size(); ++i)
            {
                const size_t next = (i + 1) % poly.size();
                game->mSpriteBatch->DrawLine(
                    static_cast<int>(std::lround(mPosition.x + poly[i].x)),
                    static_cast<int>(std::lround(mPosition.y + poly[i].y)),
                    static_cast<int>(std::lround(mPosition.x + poly[next].x)),
                    static_cast<int>(std::lround(mPosition.y + poly[next].y)),
                    Colors::Green);
            }
        }

    protected:
        SDL_FPoint mPosition = {.x = 0.0F, .y = 0.0F};
    };
} // namespace nuvelocity

#endif // NVE_COLLIDABLE2D_H
