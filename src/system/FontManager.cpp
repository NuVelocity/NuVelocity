#include "FontManager.h"

#include <SDL3_ttf/SDL_ttf.h>

#include "AssetManager.h"
#include "Font.h"

namespace nuvelocity
{
    FontManager::~FontManager()
    {
        for (auto& [pointSize, font] : mFonts)
        {
            (void)pointSize;
            if (font != nullptr)
            {
                TTF_CloseFont(font);
            }
        }
        mFonts.clear();

        if (mInitialized)
        {
            TTF_Quit();
            mInitialized = false;
        }
    }

    bool FontManager::Initialize(char** argv)
    {
        (void)argv;

        if (WarnIfAlreadyInitialized("FontManager"))
        {
            return true;
        }

        if (!TTF_Init())
        {
            return false;
        }

        mInitialized = true;
        return true;
    }

    TTF_Font* FontManager::GetFont(int pointSize) const
    {
        if (!mInitialized)
        {
            return nullptr;
        }

        auto fontIt = mFonts.find(pointSize);
        if (fontIt != mFonts.end())
        {
            return fontIt->second;
        }

        SDL_IOStream* stream = AssetManager::Load(kFontDefaultFamily);
        if (stream == nullptr)
        {
            return nullptr;
        }

        TTF_Font* font = TTF_OpenFontIO(stream, true, static_cast<float>(pointSize));
        if (font == nullptr)
        {
            return nullptr;
        }

        mFonts.emplace(pointSize, font);
        return font;
    }

    bool FontManager::MeasureString(const std::string& text,
                                    int pointSize,
                                    int& width,
                                    int& height) const
    {
        width = 0;
        height = 0;

        TTF_Font* font = GetFont(pointSize);
        if (font == nullptr)
        {
            return false;
        }

        return TTF_GetStringSize(font, text.c_str(), 0, &width, &height);
    }

    void FontManager::DrawString(SDL_Renderer* renderer,
                                 const std::string& text,
                                 const SDL_FRect& bounds,
                                 const SDL_Color& color,
                                 int pointSize,
                                 TextAlignment alignment,
                                 bool verticalCenter,
                                 int underlineIndex) const
    {
        if (renderer == nullptr || text.empty())
        {
            return;
        }

        TTF_Font* font = GetFont(pointSize);
        if (font == nullptr)
        {
            return;
        }

        SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
        if (surface == nullptr)
        {
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        if (texture == nullptr)
        {
            return;
        }

        float textureWidth = 0.0F;
        float textureHeight = 0.0F;
        if (!SDL_GetTextureSize(texture, &textureWidth, &textureHeight))
        {
            SDL_DestroyTexture(texture);
            return;
        }

        float x = bounds.x;
        switch (alignment)
        {
        case TextAlignment::Center:
            x = bounds.x + (bounds.w - textureWidth) * 0.5F;
            break;
        case TextAlignment::Right:
            x = bounds.x + bounds.w - textureWidth;
            break;
        case TextAlignment::Left:
        default:
            break;
        }

        float y = bounds.y;
        if (verticalCenter)
        {
            y = bounds.y + (bounds.h - textureHeight) * 0.5F;
        }

        SDL_FRect target{.x = x, .y = y, .w = textureWidth, .h = textureHeight};
        SDL_RenderTexture(renderer, texture, nullptr, &target);

        if (underlineIndex >= 0 && underlineIndex < static_cast<int>(text.size()))
        {
            const std::string prefix = text.substr(0, static_cast<std::size_t>(underlineIndex));
            const std::string underlinedCharacter =
                text.substr(static_cast<std::size_t>(underlineIndex), 1);

            int prefixWidth = 0;
            int prefixHeight = 0;
            int characterWidth = 0;
            int characterHeight = 0;
            MeasureString(prefix, pointSize, prefixWidth, prefixHeight);
            MeasureString(underlinedCharacter, pointSize, characterWidth, characterHeight);

            const float lineY = target.y + SDL_max(0.0F, target.h - 2.0F);
            const float lineStartX = target.x + static_cast<float>(prefixWidth);
            const float lineEndX = lineStartX + static_cast<float>(SDL_max(1, characterWidth));
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderLine(renderer, lineStartX, lineY, lineEndX, lineY);
        }

        SDL_DestroyTexture(texture);
    }
} // namespace nuvelocity
