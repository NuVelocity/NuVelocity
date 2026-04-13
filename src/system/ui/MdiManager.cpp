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

        const SDL_FRect windowRect = window->GetRect();
        const float newX = (static_cast<float>(game->mWindowWidth) - windowRect.w) / 2.0F;
        const float newY = (static_cast<float>(game->mWindowHeight) - windowRect.h) / 2.0F;

        window->SetRect({newX, newY, windowRect.w, windowRect.h});
        AddWindow(window);
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
