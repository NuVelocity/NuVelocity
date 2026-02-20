#ifndef NVE_SCENE_H
#define NVE_SCENE_H

#include "API.h"
#include <string>

namespace nuvelocity
{
    class Game;

    class Scene
    {
    public:
        virtual ~Scene() = default;

        NVE_API virtual void Update(Game* aContext) = 0;
        NVE_API virtual void Draw(Game* aContext) = 0;

        NVE_API virtual void Load(Game* aContext) = 0;
        NVE_API virtual void Unload(Game* aContext) = 0;

        NVE_API virtual std::string GetName() const = 0;

    protected:
        Scene() = default;

    private:
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
    };
} // namespace nuvelocity

#endif // NVE_SCENE_H
