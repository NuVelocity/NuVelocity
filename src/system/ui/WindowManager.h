#ifndef NVE_UI_WINDOW_MANAGER_H
#define NVE_UI_WINDOW_MANAGER_H

#include <SDL3/SDL.h>

#include <memory>
#include <vector>

namespace nuvelocity
{
    class Game;
    class InputManager;

    class Window;

    class WindowManager
    {
    public:
        WindowManager() = default;
        ~WindowManager() = default;

        void AddWindow(const std::shared_ptr<Window>& window);
        void RemoveWindow(const std::shared_ptr<Window>& window);
        void Clear();

        void Update(InputManager& input);
        void Draw(Game* game);

        const std::vector<std::shared_ptr<Window>>& GetWindows() const;

    private:
        std::vector<std::shared_ptr<Window>> mWindows;
    };
} // namespace nuvelocity

#endif // NVE_UI_WINDOW_MANAGER_H
