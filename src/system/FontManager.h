#ifndef NVE_FONT_MANAGER_H
#define NVE_FONT_MANAGER_H

#include "API.h"
#include "Font.h"
#include "Manager.h"
#include "SpriteBatch.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace nuvelocity
{
    class FontManager : public Manager
    {
    public:
        FontManager() = default;
        ~FontManager();

        bool Initialize(char** argv) override;

        bool RegisterFont(const std::string& name, std::unique_ptr<Font>&& font);
        bool SetDefaultFont(const std::string& name);
        bool SetFallbackFont(const std::string& name);

        bool MeasureString(const std::string& text, int pointSize, int& width, int& height) const;
        bool MeasureStringWithFont(const std::string& fontName,
                                   const std::string& text,
                                   int pointSize,
                                   int& width,
                                   int& height) const;

        void DrawString(SpriteBatch* batch,
                        const std::string& text,
                        const SDL_Rect& bounds,
                        const SDL_Color& color,
                        int pointSize,
                        TextAlignment alignment = TextAlignment::Left,
                        bool verticalCenter = false,
                        int underlineIndex = -1) const;

        void DrawStringWithFont(const std::string& fontName,
                                SpriteBatch* batch,
                                const std::string& text,
                                const SDL_Rect& bounds,
                                const SDL_Color& color,
                                int pointSize,
                                TextAlignment alignment = TextAlignment::Left,
                                bool verticalCenter = false,
                                int underlineIndex = -1) const;

        void DrawStringAt(SpriteBatch* batch,
                          const std::string& text,
                          int x,
                          int y,
                          const SDL_Color& color,
                          int pointSize,
                          TextAlignment alignment = TextAlignment::Left,
                          const SDL_Rect* clipRect = nullptr,
                          int underlineIndex = -1) const;

        void DrawStringWithFontAt(const std::string& fontName,
                                  SpriteBatch* batch,
                                  const std::string& text,
                                  int x,
                                  int y,
                                  const SDL_Color& color,
                                  int pointSize,
                                  TextAlignment alignment = TextAlignment::Left,
                                  const SDL_Rect* clipRect = nullptr,
                                  int underlineIndex = -1) const;

    private:
        Font* FindFont(const std::string& name) const;
        const Font* GetActiveFont() const;

        mutable std::unordered_map<std::string, std::unique_ptr<Font>> mFonts;
        mutable Font* mDefaultFont = nullptr;
        mutable Font* mFallbackFont = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_FONT_MANAGER_H
