#ifndef NVE_GAME_COMPONENT_H
#define NVE_GAME_COMPONENT_H

#include "API.h"

namespace nuvelocity
{
    class Game;

    class GameComponent
    {
    public:
        virtual ~GameComponent() = default;

        NVE_API virtual void Update(Game* game) = 0;
        NVE_API virtual void Draw(Game* game) = 0;

    protected:
        GameComponent() = default;

    private:
        GameComponent(const GameComponent&) = delete;
        GameComponent& operator=(const GameComponent&) = delete;
    };
} // namespace nuvelocity

#endif // NVE_GAME_COMPONENT_H
