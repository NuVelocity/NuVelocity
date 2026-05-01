#ifndef NVE_LABEL_H
#define NVE_LABEL_H

#include "API.h"
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

        NVE_API void SetText(const std::string& text)
        {
            mText = text;
            InvalidateLayout();
        }
        NVE_API const std::string& GetText() const
        {
            return mText;
        }

        NVE_API void SetFont(const std::string& font)
        {
            mFont = font;
            InvalidateLayout();
        }
        NVE_API const std::string& GetFont() const
        {
            return mFont;
        }

        NVE_API void SetWrap(bool wrap)
        {
            mWrap = wrap;
            InvalidateLayout();
        }
        NVE_API bool IsWrapEnabled() const
        {
            return mWrap;
        }

        NVE_API void SetPointSize(int pointSize)
        {
            mPointSize = pointSize;
            InvalidateLayout();
        }
        NVE_API int GetPointSize() const
        {
            return mPointSize;
        }

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

    private:
        std::vector<std::string>
        BuildWrappedLines(Game* game, const std::string& text, int maxWidth) const;

        int MeasureLineHeight(Game* game) const;

        std::string mText;
        std::string mFont;
        bool mWrap = false;
        int mPointSize = 13;
    };
} // namespace nuvelocity

#endif // NVE_LABEL_H
