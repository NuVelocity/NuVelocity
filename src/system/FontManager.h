#ifndef NVE_FONT_MANAGER_H
#define NVE_FONT_MANAGER_H

#include "API.h"
#include "Manager.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <unordered_map>

namespace nuvelocity
{
    enum class TextAlignment
    {
        Left,
        Center,
        Right
    };

    class FontManager : public Manager
    {
    public:
        FontManager() = default;
        ~FontManager();

        bool Initialize(char** argv) override;

        bool MeasureString(const std::string& text, int pointSize, int& width, int& height) const;

        void DrawString(SDL_Renderer* renderer,
                        const std::string& text,
                        const SDL_FRect& bounds,
                        const SDL_Color& color,
                        int pointSize,
                        TextAlignment alignment = TextAlignment::Left,
                        bool verticalCenter = true,
                        int underlineIndex = -1) const;

    private:
        TTF_Font* GetFont(int pointSize) const;

        mutable std::unordered_map<int, TTF_Font*> mFonts;
    };
} // namespace nuvelocity

#endif // NVE_FONT_MANAGER_H
