#include "FontManager.h"
#include "SpriteBatch.h"
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

    bool FontManager::RegisterFont(const std::string& name, Font* font)
    {
        if (name.empty() || font == nullptr)
        {
            return false;
        }

        mFonts[name] = font;
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
        if (name.empty())
        {
            return nullptr;
        }

        auto fontIt = mFonts.find(name);
        if (fontIt == mFonts.end())
        {
            return nullptr;
        }

        return fontIt->second;
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

    bool FontManager::MeasureStringWithFont(const std::string& fontName,
                                            const std::string& text,
                                            int pointSize,
                                            int& width,
                                            int& height) const
    {
        width = 0;
        height = 0;

        const Font* font = FindFont(fontName);
        if (font == nullptr)
        {
            font = GetActiveFont();
        }
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

    void FontManager::DrawString(SpriteBatch* batch,
                                 const std::string& text,
                                 const SDL_Rect& bounds,
                                 const SDL_Color& color,
                                 int pointSize,
                                 TextAlignment alignment,
                                 bool verticalCenter,
                                 int underlineIndex,
                                 const SDL_Color& underlineColor) const
    {
        if (batch == nullptr || text.empty())
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
        font->DrawString(batch,
                         text,
                         bounds,
                         color,
                         pointSize,
                         alignment,
                         verticalCenter,
                         underlineIndex,
                         underlineColor);
    }

    void FontManager::DrawStringWithFont(const std::string& fontName,
                                         SpriteBatch* batch,
                                         const std::string& text,
                                         const SDL_Rect& bounds,
                                         const SDL_Color& color,
                                         int pointSize,
                                         TextAlignment alignment,
                                         bool verticalCenter,
                                         int underlineIndex,
                                         const SDL_Color& underlineColor) const
    {
        if (batch == nullptr || text.empty())
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
        font->DrawString(batch,
                         text,
                         bounds,
                         color,
                         pointSize,
                         alignment,
                         verticalCenter,
                         underlineIndex,
                         underlineColor);
    }

    void FontManager::DrawStringAt(SpriteBatch* batch,
                                   const std::string& text,
                                   int x,
                                   int y,
                                   const SDL_Color& color,
                                   int pointSize,
                                   TextAlignment alignment,
                                   const SDL_Rect* clipRect,
                                   int underlineIndex,
                                   const SDL_Color& underlineColor) const
    {
        if (batch == nullptr || text.empty())
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
            font->DrawStringAt(batch,
                               text,
                               x,
                               y,
                               color,
                               pointSize,
                               alignment,
                               false,
                               underlineIndex,
                               underlineColor);
            return;
        }

        // Clip management is now handled through the batch
        batch->SetClipRect(clipRect);
        font->DrawStringAt(
            batch, text, x, y, color, pointSize, alignment, false, underlineIndex, underlineColor);
        batch->SetClipRect(nullptr); // Reset clip after drawing
    }

    void FontManager::DrawStringWithFontAt(const std::string& fontName,
                                           SpriteBatch* batch,
                                           const std::string& text,
                                           int x,
                                           int y,
                                           const SDL_Color& color,
                                           int pointSize,
                                           TextAlignment alignment,
                                           const SDL_Rect* clipRect,
                                           int underlineIndex,
                                           const SDL_Color& underlineColor) const
    {
        if (batch == nullptr || text.empty())
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
            font->DrawStringAt(batch,
                               text,
                               x,
                               y,
                               color,
                               pointSize,
                               alignment,
                               false,
                               underlineIndex,
                               underlineColor);
            return;
        }

        batch->SetClipRect(clipRect);
        font->DrawStringAt(
            batch, text, x, y, color, pointSize, alignment, false, underlineIndex, underlineColor);
        batch->SetClipRect(nullptr);
    }
} // namespace nuvelocity
