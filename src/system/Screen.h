#ifndef NVE_SCREEN_H
#define NVE_SCREEN_H

#include "API.h"
#include "GameComponent.h"
#include <string>

namespace nuvelocity
{
    class Game;

    class Screen : public GameComponent
    {
    public:
        virtual ~Screen() = default;

        NVE_API virtual void Update(Game* game) override = 0;
        NVE_API virtual void Draw(Game* game) override = 0;

        NVE_API virtual void Load(Game* game) = 0;
        NVE_API virtual void Unload(Game* game) = 0;

        NVE_API virtual std::string GetName() const = 0;

    protected:
        Screen() = default;

    private:
        Screen(const Screen&) = delete;
        Screen& operator=(const Screen&) = delete;
    };
} // namespace nuvelocity

#endif // NVE_SCREEN_H
