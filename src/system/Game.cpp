#include <SDL3/SDL_log.h>

#include "Game.h"

enum {
    NVE_DEFAULT_WINDOW_WIDTH = 640,
    NVE_DEFAULT_WINDOW_HEIGHT = 480
};

namespace nuvelocity
{
    Game::Game(const char* aWindowTitle, int aWidth, int aHeight)
        : mWindowTitle(aWindowTitle)
        , mWindowWidth(aWidth)
        , mWindowHeight(aHeight)
        , mInitialized(false)
        , mScene(nullptr)
    {
    }

    Game::Game(const char* aWindowTitle)
        : Game(aWindowTitle, NVE_DEFAULT_WINDOW_WIDTH, NVE_DEFAULT_WINDOW_HEIGHT)
    {
    }

    bool Game::Fail()
    {
        SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
        return false;
    }

    bool Game::Initialize(char** argv)
    {
        if (mInitialized) {
            return true;
        }

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            return Fail();
        }

        SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (mWindowResizable) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }
        mWindow = SDL_CreateWindow(
            mWindowTitle, mWindowWidth, mWindowHeight, windowFlags);
        if (mWindow == nullptr)
        {
            return Fail();
        }
    
        mRenderer = SDL_CreateRenderer(mWindow, nullptr);
        if (mRenderer == nullptr)
        {
            return Fail();
        }

        SDL_ShowWindow(mWindow);
        {
            int width;
            int height;
            int bbwidth;
            int bbheight;
            SDL_GetWindowSize(mWindow, &width, &height);
            SDL_GetWindowSizeInPixels(mWindow, &bbwidth, &bbheight);
            SDL_Log("Window size: %ix%i", width, height);
            SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
            if (width != bbwidth){
                SDL_Log("This is a highdpi environment.");
            }
        }

        SDL_SetRenderVSync(mRenderer, -1);

        mAsset = new AssetManager();
        if (!mAsset->Initialize(argv)) {
            return Fail();
        }

        mAudio = new AudioManager();
        if (!mAudio->Initialize(argv)) {
            return Fail();
        }

        mInitialized = true;
        return true;
    }

    void Game::Update()
    {
        mScene->Update(this);
    }

    void Game::Draw()
    {
        mScene->Draw(this);
    }

    void Game::SetScene(Scene* aScene)
    {
        if (mScene != nullptr) {
            mScene->Unload(this);
            delete mScene;
        }
        mScene = aScene;
        mScene->Load(this);
    }

    Scene* Game::GetScene()
    {
        return mScene;
    }

    Game::~Game()
    {
        delete mScene;
        mScene = nullptr;

        SDL_DestroyRenderer(mRenderer);
        SDL_DestroyWindow(mWindow);
        delete mAsset;
    }
} // namespace nuvelocity
