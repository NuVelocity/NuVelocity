#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "GPUSpriteBatch.h"
#include "Game.h"
#include "ObjectRegistration.h"
#include "RendererSpriteBatch.h"

constexpr std::uint16_t NVE_DEFAULT_WINDOW_WIDTH = 640;
constexpr std::uint16_t NVE_DEFAULT_WINDOW_HEIGHT = 480;

namespace nuvelocity
{
    Game::Game(const char* aWindowTitle, int aWidth, int aHeight)
            : mWindowTitle(aWindowTitle)
            , mWindowWidth(aWidth)
            , mWindowHeight(aHeight)
            , mWindow(nullptr)
            , mRenderer(nullptr)
            , mAsset(nullptr)
            , mAudio(nullptr)
            , mFont(nullptr)
            , mInitialized(false)
            , mScene(nullptr)
            , mInput(nullptr)
            , mSpriteBatch(nullptr)
            , mCursor(nullptr)
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
        if (mInitialized)
        {
            return true;
        }

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            return Fail();
        }

        mFont = new FontManager();
        if (!mFont->Initialize(argv))
        {
            delete mFont;
            mFont = nullptr;
            return Fail();
        }

        SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (mWindowResizable)
        {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }
        mWindow = SDL_CreateWindow(mWindowTitle, mWindowWidth, mWindowHeight, windowFlags);
        if (mWindow == nullptr)
        {
            return Fail();
        }

        mRenderer = SDL_CreateRenderer(mWindow, nullptr);
        if (mRenderer == nullptr)
        {
            return Fail();
        }

        SDL_GPUDevice* rendererDevice = SDL_GetGPURendererDevice(mRenderer);
        if (rendererDevice != nullptr)
        {
            mSpriteBatch = new GPUSpriteBatch(rendererDevice, mWindow);
        }
        else
        {
            mSpriteBatch = new RendererSpriteBatch(mRenderer, mWindow);
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
            if (width != bbwidth)
            {
                SDL_Log("This is a highdpi environment.");
            }
        }

        SDL_SetRenderVSync(mRenderer, -1);

        RegisterEngineObjectTypes();

        mAsset = new AssetManager();
        if (!mAsset->Initialize(argv))
        {
            return Fail();
        }

        UpdateMouseCursor();

        mAudio = new AudioManager();
        if (!mAudio->Initialize(argv))
        {
            return Fail();
        }

        mInput = new InputManager();
        if (!mInput->Initialize(argv))
        {
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

    void Game::HandleEvent(const SDL_Event& event) const
    {
        if (mInput != nullptr)
        {
            mInput->ProcessEvent(event);
        }
    }

    void Game::EndFrame() const
    {
        if (mInput != nullptr)
        {
            mInput->EndFrame();
        }
    }

    void Game::SetScene(Scene* aScene)
    {
        if (mScene != nullptr)
        {
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

    SpriteBatch* Game::GetSpriteBatch() const
    {
        return mSpriteBatch;
    }

    void Game::SetMouseCursor(std::string aSequencePath)
    {
        mCursorSequencePath = aSequencePath;
        if (mInitialized)
        {
            UpdateMouseCursor();
        }
    }

    void Game::UpdateMouseCursor()
    {
        Sequence* cursorSequence = AssetManager::LoadSequence(mCursorSequencePath);
        if (cursorSequence == nullptr || cursorSequence->GetFrameCount() == 0)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Mouse cursor sequence is unavailable at %s.",
                        mCursorSequencePath.c_str());
            return;
        }

        Frame* cursorFrame = cursorSequence->GetFrame(0);
        SDL_Surface* cursorSurface = cursorFrame != nullptr ? cursorFrame->GetSurface() : nullptr;
        if (cursorSurface == nullptr)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Mouse cursor surface could not be created from sequence frame.");
            return;
        }

        // FIXME: The hot spot is currently hardcoded. This should be taken from sequence data.
        SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 18, 22);
        if (cursor == nullptr)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Failed to create custom mouse cursor: %s",
                        SDL_GetError());
            return;
        }

        if (mCursor != nullptr)
        {
            SDL_DestroyCursor(mCursor);
        }

        mCursor = cursor;
        SDL_SetCursor(mCursor);
    }

    Game::~Game()
    {
        delete mScene;
        mScene = nullptr;

        if (mCursor != nullptr)
        {
            SDL_DestroyCursor(mCursor);
            mCursor = nullptr;
        }

        delete mSpriteBatch;
        mSpriteBatch = nullptr;

        delete mInput;
        mInput = nullptr;

        SDL_DestroyRenderer(mRenderer);
        SDL_DestroyWindow(mWindow);
        delete mFont;
        mFont = nullptr;
        delete mAudio;
        delete mAsset;
    }
} // namespace nuvelocity
