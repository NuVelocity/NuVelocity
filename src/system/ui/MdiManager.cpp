#include "MdiManager.h"
#include "Game.h"
#include "MdiWindow.h"
#include <algorithm>
#include <system/AssetManager.h>
#include <system/InputManager.h>
#include <system/ui/skin/JWindowClassicTheme.h>
#include <system/ui/skin/JWindowSkin.h>

namespace nuvelocity
{
    MdiManager::~MdiManager()
    {
        for (auto& pair : mSkins)
        {
            delete pair.second;
        }
        mSkins.clear();
    }

    bool MdiManager::Initialize(char** argv)
    {
        (void)argv;

        // Register default Classic skin
        auto* classic = new JWindowClassicTheme();
        RegisterSkin("!None", classic);
        SetDefaultSkin(classic);

        mInitialized = true;
        return true;
    }

    void MdiManager::AddWindow(const std::shared_ptr<MdiWindow>& window)
    {
        if (window != nullptr)
        {
            mWindows.push_back(window);
            SetActiveWindow(window);
        }
    }

    void MdiManager::AddCenteredWindow(Game* game, const std::shared_ptr<MdiWindow>& window)
    {
        if (game == nullptr || window == nullptr)
        {
            return;
        }

        const SDL_Rect windowRect = window->GetRect();
        const int newX = (game->mWindowWidth - windowRect.w) / 2;
        const int newY = (game->mWindowHeight - windowRect.h) / 2;

        window->SetRect({.x = newX, .y = newY, .w = windowRect.w, .h = windowRect.h});
        AddWindow(window);
    }

    void MdiManager::RemoveWindow(const std::shared_ptr<MdiWindow>& window)
    {
        auto [first, last] = std::ranges::remove(mWindows, window);
        mWindows.erase(first, last);
    }

    void MdiManager::Clear()
    {
        mWindows.clear();
    }

    void MdiManager::Update(Game* game)
    {
        if (game == nullptr || game->mInput == nullptr)
        {
            return;
        }

        InputManager& input = *game->mInput;
        const SDL_Point mousePosition = input.GetMousePosition();

        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            for (std::size_t index = mWindows.size(); index > 0; --index)
            {
                std::shared_ptr<MdiWindow>& window = mWindows[index - 1];
                if (window != nullptr && window->IsVisible() && window->Intersects(mousePosition))
                {
                    SetActiveWindow(window);
                    break;
                }
            }
        }

        std::shared_ptr<MdiWindow> activeWindow = GetActiveWindow();
        bool mouseOverAnyWindow = false;
        for (auto& window : mWindows)
        {
            if (window == nullptr)
            {
                continue;
            }

            const bool isActive = (window == activeWindow);
            window->SetActive(isActive);

            if (isActive)
            {
                window->Update(game);
            }

            if (window->IsVisible() && window->Intersects(mousePosition))
            {
                mouseOverAnyWindow = true;
            }
        }

        // Consume mouse pressed/released transitions when the pointer is inside
        // any visible window so the scene does not receive the same click.
        // We only clear pressed/released (not down) to keep drag logic intact.
        if (mouseOverAnyWindow)
        {
            input.ConsumeMouseButton(SDL_BUTTON_LEFT);
            input.ConsumeMouseButton(SDL_BUTTON_RIGHT);
        }

        if (input.IsKeyPressed(SDL_SCANCODE_ESCAPE))
        {
            if (activeWindow != nullptr)
            {
                input.ConsumeKey(SDL_SCANCODE_ESCAPE);
                activeWindow->Close();
            }
        }

        auto [rmFirst, rmLast] =
            std::ranges::remove_if(mWindows,
                                   [](const std::shared_ptr<MdiWindow>& window)
                                   { return window == nullptr || window->ShouldClose(); });
        mWindows.erase(rmFirst, rmLast);
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

    std::shared_ptr<MdiWindow> MdiManager::GetActiveWindow() const
    {
        return mWindows.empty() ? nullptr : mWindows.back();
    }

    void MdiManager::SetActiveWindow(const std::shared_ptr<MdiWindow>& window)
    {
        if (window == nullptr)
        {
            return;
        }

        auto it = std::ranges::find(mWindows, window);
        if (it != mWindows.end())
        {
            std::shared_ptr<MdiWindow> active = *it;
            mWindows.erase(it);
            mWindows.push_back(active);
        }
    }

    void MdiManager::RegisterSkin(const std::string& name, JWindowSkin* skin)
    {
        if (skin != nullptr)
        {
            mSkins[name] = skin;
            SDL_Log("Registered skin '%s'", name.c_str());
        }
    }

    JWindowSkin* MdiManager::GetSkin(const std::string& name) const
    {
        auto it = mSkins.find(name);
        if (it != mSkins.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void MdiManager::SetDefaultSkin(JWindowSkin* skin)
    {
        mDefaultSkin = skin;
    }

    JWindowSkin* MdiManager::GetDefaultSkin() const
    {
        return mDefaultSkin;
    }
} // namespace nuvelocity
