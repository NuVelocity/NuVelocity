#ifndef NVE_LABEL_H
#define NVE_LABEL_H

#include "API.h"
#include "Colors.h"
#include "TextAlignment.h"
#include "Widget.h"
#include <string>
#include <vector>

namespace nuvelocity
{
    class Label : public Widget
    {
    public:
        NVE_API Label(std::string text = "", std::string font = "OCR");
        NVE_API virtual ~Label() = default;

        NVE_API void SetText(const std::string& text);
        NVE_API const std::string& GetText() const;

        NVE_API void SetFont(const std::string& font);
        NVE_API const std::string& GetFont() const;

        NVE_API void SetWrap(bool wrap);
        NVE_API bool IsWrapEnabled() const;

        NVE_API void SetPointSize(int pointSize);
        NVE_API int GetPointSize() const;
        NVE_API void SetAlignment(TextAlignment alignment);
        NVE_API TextAlignment GetAlignment() const;
        NVE_API void SetColor(const SDL_Color& color);
        NVE_API const SDL_Color& GetColor() const;
        NVE_API void SetVerticalCenter(bool center);
        NVE_API bool IsVerticalCenter() const;

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;
        NVE_API int GetRequiredHeight(Game* game, int maxWidth) const;

    protected:
        NVE_API void InvalidateLayout() override;

    private:
        std::vector<std::string> GetWrappedLines(Game* game, int maxWidth) const;

        std::vector<std::string>
        BuildWrappedLines(Game* game, const std::string& text, int maxWidth) const;

        int MeasureLineHeight(Game* game) const;

        std::string mText;
        std::string mFont;
        bool mWrap = false;
        int mPointSize = 13;
        TextAlignment mAlignment = TextAlignment::Left;
        bool mVerticalCenter = false;
        SDL_Color mColor = Colors::White;

        mutable std::vector<std::string> mCachedLines;
        mutable int mLastWrapWidth = -1;
        mutable std::string mLastWrapText;
    };
} // namespace nuvelocity

#endif // NVE_LABEL_H
