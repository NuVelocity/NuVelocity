#include "FontManager.h"
#include "Utils.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <memory>

namespace nuvelocity
{
    FontManager::~FontManager()
    {
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

    bool FontManager::RegisterFont(const std::string& name, std::unique_ptr<Font>&& font)
    {
        if (name.empty() || font == nullptr)
        {
            return false;
        }

        mFonts[name] = std::move(font);
        return true;
    }

    bool FontManager::SetDefaultFont(const std::string& name)
    {
        Font* font = FindFont(name);
        if (font == nullptr)
        {
            return false;
        }

        mDefaultFont = font;
        return true;
    }

    bool FontManager::SetFallbackFont(const std::string& name)
    {
        Font* font = FindFont(name);
        if (font == nullptr)
        {
            return false;
        }

        mFallbackFont = font;
        return true;
    }

    Font* FontManager::FindFont(const std::string& name) const
    {
        auto fontIt = mFonts.find(name);
        if (fontIt == mFonts.end())
        {
            return nullptr;
        }

        return fontIt->second.get();
    }

    const Font* FontManager::GetActiveFont() const
    {
        if (mDefaultFont != nullptr)
        {
            return mDefaultFont;
        }

        return mFallbackFont;
    }

    bool FontManager::MeasureString(const std::string& text,
                                    int pointSize,
                                    int& width,
                                    int& height) const
    {
        width = 0;
        height = 0;

        const Font* font = GetActiveFont();
        if (font == nullptr)
        {
            return false;
        }

        if (font->MeasureString(text, pointSize, width, height))
        {
            return true;
        }

        if (font != mFallbackFont && mFallbackFont != nullptr)
        {
            return mFallbackFont->MeasureString(text, pointSize, width, height);
        }

        return false;
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

        const Font* font = GetActiveFont();
        if (font == nullptr)
        {
            return;
        }

        int width = 0;
        int height = 0;
        if (!font->MeasureString(text, pointSize, width, height) && font != mFallbackFont &&
            mFallbackFont != nullptr)
        {
            font = mFallbackFont;
        }
        font->DrawString(
            renderer, text, bounds, color, pointSize, alignment, verticalCenter, underlineIndex);
    }

    void FontManager::DrawStringWithFont(const std::string& fontName,
                                         SDL_Renderer* renderer,
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

        const Font* font = FindFont(fontName);
        if (font == nullptr)
        {
            font = GetActiveFont();
        }
        if (font == nullptr)
        {
            return;
        }

        int width = 0;
        int height = 0;
        if (!font->MeasureString(text, pointSize, width, height) && font != mFallbackFont &&
            mFallbackFont != nullptr)
        {
            font = mFallbackFont;
        }
        font->DrawString(
            renderer, text, bounds, color, pointSize, alignment, verticalCenter, underlineIndex);
    }

    void FontManager::DrawStringAt(SDL_Renderer* renderer,
                                   const std::string& text,
                                   float x,
                                   float y,
                                   const SDL_Color& color,
                                   int pointSize,
                                   TextAlignment alignment,
                                   const SDL_Rect* clipRect,
                                   int underlineIndex) const
    {
        if (renderer == nullptr || text.empty())
        {
            return;
        }

        const Font* font = GetActiveFont();
        if (font == nullptr)
        {
            return;
        }

        int width = 0;
        int height = 0;
        if (!font->MeasureString(text, pointSize, width, height) && font != mFallbackFont &&
            mFallbackFont != nullptr)
        {
            font = mFallbackFont;
        }

        if (clipRect == nullptr)
        {
            font->DrawStringAt(
                renderer, text, x, y, color, pointSize, alignment, false, underlineIndex);
            return;
        }

        SDL_Rect previousClip{};
        const bool hadClip = SDL_GetRenderClipRect(renderer, &previousClip);

        SDL_Rect activeClip = *clipRect;
        if (hadClip && !intersects(previousClip, *clipRect, activeClip))
        {
            return;
        }

        SDL_SetRenderClipRect(renderer, &activeClip);
        font->DrawStringAt(
            renderer, text, x, y, color, pointSize, alignment, false, underlineIndex);
        SDL_SetRenderClipRect(renderer, hadClip ? &previousClip : nullptr);
    }

    void FontManager::DrawStringWithFontAt(const std::string& fontName,
                                           SDL_Renderer* renderer,
                                           const std::string& text,
                                           float x,
                                           float y,
                                           const SDL_Color& color,
                                           int pointSize,
                                           TextAlignment alignment,
                                           const SDL_Rect* clipRect,
                                           int underlineIndex) const
    {
        if (renderer == nullptr || text.empty())
        {
            return;
        }

        const Font* font = FindFont(fontName);
        if (font == nullptr)
        {
            font = GetActiveFont();
        }
        if (font == nullptr)
        {
            return;
        }

        if (clipRect == nullptr)
        {
            font->DrawStringAt(
                renderer, text, x, y, color, pointSize, alignment, false, underlineIndex);
            return;
        }

        SDL_Rect previousClip{};
        const bool hadClip = SDL_GetRenderClipRect(renderer, &previousClip);

        SDL_Rect activeClip = *clipRect;
        if (hadClip && !intersects(previousClip, *clipRect, activeClip))
        {
            return;
        }

        SDL_SetRenderClipRect(renderer, &activeClip);
        font->DrawStringAt(
            renderer, text, x, y, color, pointSize, alignment, false, underlineIndex);
        SDL_SetRenderClipRect(renderer, hadClip ? &previousClip : nullptr);
    }
} // namespace nuvelocity
