#ifndef NVE_FONT_MANAGER_H
#define NVE_FONT_MANAGER_H

#include "API.h"
#include "Colors.h"
#include "Font.h"
#include "Manager.h"
#include "TextAlignment.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace nuvelocity
{
    class SpriteBatch;

    class FontManager : public Manager
    {
    public:
        NVE_API FontManager() = default;
        NVE_API ~FontManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API bool RegisterFont(const std::string& name, Font* font);
        NVE_API bool SetDefaultFont(const std::string& name);
        NVE_API bool SetFallbackFont(const std::string& name);

        NVE_API bool
        MeasureString(const std::string& text, int pointSize, int& width, int& height) const;
        NVE_API bool MeasureStringWithFont(const std::string& fontName,
                                           const std::string& text,
                                           int pointSize,
                                           int& width,
                                           int& height) const;

        NVE_API void DrawString(SpriteBatch* batch,
                                const std::string& text,
                                const SDL_Rect& bounds,
                                const SDL_Color& color,
                                int pointSize,
                                TextAlignment alignment = TextAlignment::Left,
                                bool verticalCenter = false,
                                int underlineIndex = -1,
                                const SDL_Color& underlineColor = Colors::White) const;

        NVE_API void DrawStringWithFont(const std::string& fontName,
                                        SpriteBatch* batch,
                                        const std::string& text,
                                        const SDL_Rect& bounds,
                                        const SDL_Color& color,
                                        int pointSize,
                                        TextAlignment alignment = TextAlignment::Left,
                                        bool verticalCenter = false,
                                        int underlineIndex = -1,
                                        const SDL_Color& underlineColor = Colors::White) const;

        NVE_API void DrawStringAt(SpriteBatch* batch,
                                  const std::string& text,
                                  int x,
                                  int y,
                                  const SDL_Color& color,
                                  int pointSize,
                                  TextAlignment alignment = TextAlignment::Left,
                                  const SDL_Rect* clipRect = nullptr,
                                  int underlineIndex = -1,
                                  const SDL_Color& underlineColor = Colors::White) const;

        NVE_API void DrawStringWithFontAt(const std::string& fontName,
                                          SpriteBatch* batch,
                                          const std::string& text,
                                          int x,
                                          int y,
                                          const SDL_Color& color,
                                          int pointSize,
                                          TextAlignment alignment = TextAlignment::Left,
                                          const SDL_Rect* clipRect = nullptr,
                                          int underlineIndex = -1,
                                          const SDL_Color& underlineColor = Colors::White) const;

    private:
        Font* FindFont(const std::string& name) const;
        const Font* GetActiveFont() const;

        mutable std::unordered_map<std::string, Font*> mFonts;
        mutable Font* mDefaultFont = nullptr;
        mutable Font* mFallbackFont = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_FONT_MANAGER_H
