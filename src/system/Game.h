#ifndef NVE_GAME_H
#define NVE_GAME_H

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>

#include "API.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "FontManager.h"
#include "InputManager.h"
#include "Scene.h"

namespace nuvelocity
{
    class SpriteBatch;

    class Game
    {
    public:
        SDL_AppResult mAppResult = SDL_APP_CONTINUE;
        SDL_Window* mWindow;
        SDL_Renderer* mRenderer;

        const char* mWindowTitle;
        bool mWindowResizable = false;
        int mWindowWidth;
        int mWindowHeight;

        AssetManager* mAsset;
        AudioManager* mAudio;
        FontManager* mFont;
        InputManager* mInput;
        SpriteBatch* mSpriteBatch;

        NVE_API Game(const char* aWindowTitle, int aWidth, int aHeight);
        NVE_API Game(const char* aWindowTitle);
        NVE_API ~Game();

        NVE_API bool Initialize(char** argv);

        NVE_API void Update();
        NVE_API void Draw();
        NVE_API void HandleEvent(const SDL_Event& event) const;
        NVE_API void EndFrame() const;

        NVE_API void SetScene(Scene* aScene);
        NVE_API Scene* GetScene();
        NVE_API SpriteBatch* GetSpriteBatch() const;

    protected:
        Scene* mScene;
        static inline bool Fail();

    private:
        bool mInitialized;
    };
} // namespace nuvelocity

#endif // NVE_GAME_H
