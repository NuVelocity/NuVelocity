#include "MdiManager.h"
#include "Game.h"
#include "MdiWindow.h"
#include <algorithm>
#include <system/InputManager.h>

namespace nuvelocity
{
    bool MdiManager::Initialize(char** argv)
    {
        (void)argv;
        mInitialized = true;
        return true;
    }

    void MdiManager::AddWindow(const std::shared_ptr<MdiWindow>& window)
    {
        if (window != nullptr)
        {
            mWindows.push_back(window);
        }
    }

    void MdiManager::RemoveWindow(const std::shared_ptr<MdiWindow>& window)
    {
        mWindows.erase(std::remove(mWindows.begin(), mWindows.end(), window), mWindows.end());
    }

    void MdiManager::Clear()
    {
        mWindows.clear();
    }

    void MdiManager::Update(Game* aGame)
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
                std::shared_ptr<MdiWindow>& window = mWindows[index - 1];
                if (window != nullptr && window->IsVisible() && window->Intersects(mousePosition))
                {
                    std::shared_ptr<MdiWindow> focusedWindow = window;
                    mWindows.erase(mWindows.begin() + static_cast<long>(index - 1));
                    mWindows.push_back(focusedWindow);
                    break;
                }
            }
        }

        for (std::size_t index = 0; index < mWindows.size(); ++index)
        {
            std::shared_ptr<MdiWindow>& window = mWindows[index];
            if (window == nullptr)
            {
                continue;
            }

            window->SetActive(index == mWindows.size() - 1);
            window->Update(aGame);
        }

        mWindows.erase(std::remove_if(mWindows.begin(),
                                      mWindows.end(),
                                      [](const std::shared_ptr<MdiWindow>& window)
                                      { return window == nullptr || window->ShouldClose(); }),
                       mWindows.end());
    }

    void MdiManager::Draw(Game* game)
    {
        for (const std::shared_ptr<MdiWindow>& window : mWindows)
        {
            if (window != nullptr)
            {
                window->Draw(game);
            }
        }
    }

    const std::vector<std::shared_ptr<MdiWindow>>& MdiManager::GetWindows() const
    {
        return mWindows;
    }
} // namespace nuvelocity
