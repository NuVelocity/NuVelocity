#ifndef NVE_UI_MDI_MANAGER_H
#define NVE_UI_MDI_MANAGER_H

#include "Manager.h"

#include <SDL3/SDL.h>
#include <memory>
#include <vector>

namespace nuvelocity
{
    class Game;
    class InputManager;

    class MdiWindow;

    class MdiManager : public Manager
    {
    public:
        MdiManager() = default;
        ~MdiManager() override = default;

        bool Initialize(char** argv) override;

        void AddWindow(const std::shared_ptr<MdiWindow>& window);
        void RemoveWindow(const std::shared_ptr<MdiWindow>& window);
        void Clear();

        void Update(Game* aGame);
        void Draw(Game* game);

        const std::vector<std::shared_ptr<MdiWindow>>& GetWindows() const;

    private:
        std::vector<std::shared_ptr<MdiWindow>> mWindows;
    };
} // namespace nuvelocity

#endif // NVE_UI_MDI_MANAGER_H
