#ifndef NVE_RENDERER_SPRITE_BATCH_H
#define NVE_RENDERER_SPRITE_BATCH_H

#include <SDL3/SDL.h>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "SpriteBatch.h"

namespace nuvelocity
{

    // SDL Renderer-based sprite batch implementation
    class RendererSpriteBatch : public SpriteBatch
    {
    public:
        RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window);
        ~RendererSpriteBatch() override;

        RendererSpriteBatch(const RendererSpriteBatch&) = delete;
        RendererSpriteBatch& operator=(const RendererSpriteBatch&) = delete;

        void Draw(SDL_Surface* surface,
                  const SDL_Rect* destRect = nullptr,
                  const SDL_Rect* srcRect = nullptr,
                  SDL_Color color = {255, 255, 255, 255}) override;

        void DrawCentered(SDL_Surface* surface) override;

        void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness = 1) override;

        void FillRect(const SDL_Rect* rect, SDL_Color color) override;

        void SetClipRect(const SDL_Rect* rect) override;

        void Clear(SDL_Color color) override;

        void Flush() override;

        void Present() override;

    private:
        enum class CommandType : uint8_t
        {
            DrawSurface,
            DrawTexture,
            DrawLine,
            FillRect,
            SetClipRect,
            Clear
        };

        struct DrawCommand
        {
            CommandType type;
            SDL_Surface* surface;
            SDL_Texture* texture;
            bool centered;
            bool hasDestRect;
            SDL_Rect destRect;
            bool hasSrcRect;
            SDL_Rect srcRect;
            bool hasClipRect;
            SDL_Rect clipRect;
            SDL_Color color;
            int x1, y1, x2, y2;
            int thickness;
            SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
        };

        SDL_Renderer* mRenderer;
        SDL_Window* mWindow;
        std::vector<DrawCommand> mDrawCommands;
        std::unordered_map<SDL_Surface*, SDL_Texture*> mTextureCache;
    };
} // namespace nuvelocity

#endif // NVE_RENDERER_SPRITE_BATCH_H
