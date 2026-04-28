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
        mWindows.erase(std::ranges::remove(mWindows, window), mWindows.end());
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
            window->Update(game);
        }

        if (input.IsKeyPressed(SDL_SCANCODE_ESCAPE))
        {
            if (!mWindows.empty())
            {
                mWindows.back()->Close();
            }
        }

        mWindows.erase(
            std::ranges::remove_if(mWindows,

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
