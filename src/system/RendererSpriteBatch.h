#ifndef NVE_RENDERER_SPRITE_BATCH_H
#define NVE_RENDERER_SPRITE_BATCH_H

#include <SDL3/SDL.h>
#include <vector>

#include "SpriteBatch.h"

namespace nuvelocity
{

    // SDL Renderer-based sprite batch implementation
    class RendererSpriteBatch : public SpriteBatch
    {
    public:
        RendererSpriteBatch(SDL_Renderer* renderer, SDL_Window* window);
        ~RendererSpriteBatch() override = default;

        RendererSpriteBatch(const RendererSpriteBatch&) = delete;
        RendererSpriteBatch& operator=(const RendererSpriteBatch&) = delete;

        void Draw(SDL_Surface* surface,
                  const SDL_FRect* destRect = nullptr,
                  const SDL_FRect* srcRect = nullptr,
                  SDL_Color color = {255, 255, 255, 255}) override;

        void DrawCentered(SDL_Surface* surface) override;

        void DrawLine(float x1,
                      float y1,
                      float x2,
                      float y2,
                      SDL_Color color,
                      float thickness = 1.0f) override;

        void FillRect(const SDL_FRect* rect, SDL_Color color) override;

        void SetClipRect(const SDL_Rect* rect) override;

        void Clear(SDL_Color color) override;

        void Flush() override;

        void Present() override;

    private:
        enum class CommandType
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
            SDL_FRect destRect;
            bool hasSrcRect;
            SDL_FRect srcRect;
            bool hasClipRect;
            SDL_Rect clipRect;
            SDL_Color color;
            float x1, y1, x2, y2;
            float thickness;
            SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
        };

        SDL_Renderer* mRenderer;
        SDL_Window* mWindow;
        std::vector<DrawCommand> mDrawCommands;
    };
} // namespace nuvelocity

#endif // NVE_RENDERER_SPRITE_BATCH_H
