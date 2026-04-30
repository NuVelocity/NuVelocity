#ifndef NVE_UI_MDI_MANAGER_H
#define NVE_UI_MDI_MANAGER_H

#include "API.h"
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
        NVE_API MdiManager() = default;
        NVE_API ~MdiManager() override;

        NVE_API bool Initialize(char** argv) override;

        NVE_API void AddWindow(const std::shared_ptr<MdiWindow>& window);
        NVE_API void AddCenteredWindow(Game* game, const std::shared_ptr<MdiWindow>& window);
        NVE_API void RemoveWindow(const std::shared_ptr<MdiWindow>& window);
        NVE_API void Clear();

        NVE_API void Update(Game* game);
        NVE_API void Draw(Game* game);

        NVE_API const std::vector<std::shared_ptr<MdiWindow>>& GetWindows() const;
        NVE_API std::shared_ptr<MdiWindow> GetActiveWindow() const;
        NVE_API void SetActiveWindow(const std::shared_ptr<MdiWindow>& window);

        NVE_API void RegisterSkin(const std::string& name, JWindowSkin* skin);
        NVE_API JWindowSkin* GetSkin(const std::string& name) const;

        NVE_API void SetDefaultSkin(JWindowSkin* skin);
        NVE_API JWindowSkin* GetDefaultSkin() const;

    private:
        std::vector<std::shared_ptr<MdiWindow>> mWindows;
        std::unordered_map<std::string, JWindowSkin*> mSkins;
        JWindowSkin* mDefaultSkin = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_UI_MDI_MANAGER_H
