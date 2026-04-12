#include "WindowManager.h"
#include "Game.h"
#include "Window.h"
#include <algorithm>
#include <system/InputManager.h>

namespace nuvelocity
{
    void WindowManager::AddWindow(const std::shared_ptr<Window>& window)
    {
        if (window != nullptr)
        {
            mWindows.push_back(window);
        }
    }

    void WindowManager::RemoveWindow(const std::shared_ptr<Window>& window)
    {
        mWindows.erase(std::remove(mWindows.begin(), mWindows.end(), window), mWindows.end());
    }

    void WindowManager::Clear()
    {
        mWindows.clear();
    }

    void WindowManager::Update(Game* aGame)
    {
        if (aGame == nullptr || aGame->mInput == nullptr)
        {
            return;
        }

        InputManager& input = *aGame->mInput;
        const SDL_FPoint mousePosition = input.GetMousePosition();

        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            for (std::size_t index = mWindows.size(); index > 0; --index)
            {
                std::shared_ptr<Window>& window = mWindows[index - 1];
                if (window != nullptr && window->IsVisible() && window->Intersects(mousePosition))
                {
                    std::shared_ptr<Window> focusedWindow = window;
                    mWindows.erase(mWindows.begin() + static_cast<long>(index - 1));
                    mWindows.push_back(focusedWindow);
                    break;
                }
            }
        }

        for (std::size_t index = 0; index < mWindows.size(); ++index)
        {
            std::shared_ptr<Window>& window = mWindows[index];
            if (window == nullptr)
            {
                continue;
            }

            window->SetActive(index == mWindows.size() - 1);
            window->Update(aGame);
        }

        mWindows.erase(std::remove_if(mWindows.begin(),
                                      mWindows.end(),
                                      [](const std::shared_ptr<Window>& window)
                                      { return window == nullptr || window->ShouldClose(); }),
                       mWindows.end());
    }

    void WindowManager::Draw(Game* game)
    {
        for (const std::shared_ptr<Window>& window : mWindows)
        {
            if (window != nullptr)
            {
                window->Draw(game);
            }
        }
    }

    const std::vector<std::shared_ptr<Window>>& WindowManager::GetWindows() const
    {
        return mWindows;
    }
} // namespace nuvelocity
