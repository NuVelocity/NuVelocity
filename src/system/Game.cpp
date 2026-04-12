#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>

#include "Font.h"
#include "FontBitmap.h"
#ifdef NVE_GPU_SUPPORT
#include "GPUSpriteBatch.h"
#endif
#include "Game.h"
#include "ObjectRegistration.h"
#include "RendererSpriteBatch.h"

constexpr std::uint16_t NVE_DEFAULT_WINDOW_WIDTH = 640;
constexpr std::uint16_t NVE_DEFAULT_WINDOW_HEIGHT = 480;

namespace nuvelocity
{
    static bool RegisterEngineDefaultFonts(FontManager& fontManager)
    {
        constexpr const char* kDefaultFontName = "Default";
        constexpr const char* kDefaultFontPath = "Resources/Fonts/!None.font.txt";
        constexpr const char* kBitmapFallbackName = "!None";
        constexpr const char* kBitmapFallbackPath = "Fonts/!None";

        bool hasAnyFont = false;

        Font* defaultFontRaw = AssetManager::LoadFont(kDefaultFontPath);
        if (defaultFontRaw != nullptr)
        {
            auto defaultFont = std::unique_ptr<Font>(defaultFontRaw);
            if (fontManager.RegisterFont(kDefaultFontName, std::move(defaultFont)))
            {
                fontManager.SetDefaultFont(kDefaultFontName);
                hasAnyFont = true;
            }
        }

        FontBitmap* fallbackBitmap = AssetManager::LoadFontBitmap(kBitmapFallbackPath);
        if (fallbackBitmap != nullptr)
        {
            auto fallbackFont = std::unique_ptr<Font>(static_cast<Font*>(fallbackBitmap));
            if (fontManager.RegisterFont(kBitmapFallbackName, std::move(fallbackFont)))
            {
                fontManager.SetFallbackFont(kBitmapFallbackName);
                hasAnyFont = true;
                if (defaultFontRaw == nullptr)
                {
                    fontManager.SetDefaultFont(kBitmapFallbackName);
                }
            }
        }

        return hasAnyFont;
    }

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
            , mGPUDevice(nullptr)
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

        RegisterEngineObjectTypes();

        mAsset = new AssetManager();
        if (!mAsset->Initialize(argv))
        {
            return Fail();
        }

        UpdateMouseCursor();

        mModuleInfo =
            mModuleInfoPath.empty()
                ? new ModuleInfo()
                : static_cast<ModuleInfo*>(AssetManager::LoadPropertyFile(mModuleInfoPath));

        SDL_SetAppMetadata(mModuleInfo->GetModuleName().c_str(),
                           mModuleInfo->GetModuleVersion().c_str(),
                           mModuleInfo->GetModuleId().c_str());

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
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

#ifdef NVE_GPU_SUPPORT
        SDL_GPUShaderFormat shaderFormats =
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
        mGPUDevice = SDL_CreateGPUDevice(shaderFormats, false, nullptr);

        if (mGPUDevice == nullptr)
        {
            mRenderer = SDL_CreateRenderer(mWindow, nullptr);
            if (mRenderer == nullptr)
            {
                return Fail();
            }

            // Fallback to standard RendererSpriteBatch if GPU init failed
            if (mSpriteBatch == nullptr)
            {
                mSpriteBatch = new RendererSpriteBatch(mRenderer, mWindow);
            }
        }
        else
        {
            if (SDL_ClaimWindowForGPUDevice(mGPUDevice, mWindow))
            {
                SDL_Log("Using %s GPU implementation.", SDL_GetGPUDeviceDriver(mGPUDevice));
                mSpriteBatch = new GPUSpriteBatch(mGPUDevice, mWindow);
            }
            else
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "SDL_ClaimWindowForGPUDevice failed: %s",
                            SDL_GetError());
                SDL_DestroyGPUDevice(mGPUDevice);
                mGPUDevice = nullptr;
            }
        }
#else
        mRenderer = SDL_CreateRenderer(mWindow, nullptr);
        if (mRenderer == nullptr)
        {
            return Fail();
        }

        mSpriteBatch = new RendererSpriteBatch(mRenderer, mWindow);
#endif

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

        if (mRenderer)
        {
            SDL_SetRenderVSync(mRenderer, -1);
        }

        mFont = new FontManager();
        if (!mFont->Initialize(argv))
        {
            delete mFont;
            mFont = nullptr;
            return Fail();
        }

        (void)RegisterEngineDefaultFonts(*mFont);

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
        SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0);
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

    void Game::SetModuleInfo(std::string aModuleInfoPath)
    {
        mModuleInfoPath = aModuleInfoPath;
        if (mInitialized)
        {
            throw std::runtime_error(
                "Changing module info path after initialization is not supported.");
        }
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

        // mFont and mAsset are freed before GPU device/window teardown so any
        // GPU-adjacent cleanup they trigger (e.g. surface frees) happens while
        // the device is still alive.
        delete mFont;
        mFont = nullptr;
        delete mAudio;
        delete mAsset;

        if (mRenderer != nullptr)
        {
            SDL_DestroyRenderer(mRenderer);
            SDL_DestroyWindow(mWindow);
        }
        if (mGPUDevice != nullptr)
        {
            if (mWindow != nullptr)
            {
                SDL_ReleaseWindowFromGPUDevice(mGPUDevice, mWindow);
                SDL_DestroyWindow(mWindow);
            }
            SDL_DestroyGPUDevice(mGPUDevice);
            mGPUDevice = nullptr;
        }
    }
} // namespace nuvelocity
