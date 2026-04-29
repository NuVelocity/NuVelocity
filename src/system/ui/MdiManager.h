#ifndef NVE_UI_MDI_MANAGER_H
#define NVE_UI_MDI_MANAGER_H

#include "Manager.h"

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    class Game;
    class InputManager;

    class MdiWindow;
    class JWindowSkin;

    class MdiManager : public Manager
    {
    public:
        MdiManager() = default;
        ~MdiManager() override;

        bool Initialize(char** argv) override;

        void AddWindow(const std::shared_ptr<MdiWindow>& window);
        void AddCenteredWindow(Game* game, const std::shared_ptr<MdiWindow>& window);
        void RemoveWindow(const std::shared_ptr<MdiWindow>& window);
        void Clear();

        void Update(Game* game);
        void Draw(Game* game);

        const std::vector<std::shared_ptr<MdiWindow>>& GetWindows() const;
        std::shared_ptr<MdiWindow> GetActiveWindow() const;
        void SetActiveWindow(const std::shared_ptr<MdiWindow>& window);

        void RegisterSkin(const std::string& name, JWindowSkin* skin);
        JWindowSkin* GetSkin(const std::string& name) const;

        void SetDefaultSkin(JWindowSkin* skin);
        JWindowSkin* GetDefaultSkin() const;

    private:
        std::vector<std::shared_ptr<MdiWindow>> mWindows;
        std::unordered_map<std::string, JWindowSkin*> mSkins;
        JWindowSkin* mDefaultSkin = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_UI_MDI_MANAGER_H
