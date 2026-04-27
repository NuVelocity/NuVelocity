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

        virtual std::vector<SDL_FPoint> GetCollisionPolygon() const;

        virtual void OnHit() {}

        virtual bool IsDestroyed() const
        {
            return false;
        }

        void Draw(const Game* game) const
        {
            if (!game->mSpriteBatch->IsDrawBoundsEnabled())
            {
                return;
            }

            for (size_t i = 0; i < GetCollisionPolygon().size(); ++i)
            {
                const size_t next = (i + 1) % GetCollisionPolygon().size();
                game->mSpriteBatch->DrawLine(
                    static_cast<int>(std::lround(mPosition.x + GetCollisionPolygon()[i].x)),
                    static_cast<int>(std::lround(mPosition.y + GetCollisionPolygon()[i].y)),
                    static_cast<int>(std::lround(mPosition.x + GetCollisionPolygon()[next].x)),
                    static_cast<int>(std::lround(mPosition.y + GetCollisionPolygon()[next].y)),
                    Colors::Green);
            }
        }

    protected:
        SDL_FPoint mPosition = {.x = 0.0F, .y = 0.0F};
    };
} // namespace nuvelocity

#endif // NVE_COLLIDABLE2D_H
