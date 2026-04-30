#ifndef NVE_LABEL_H
#define NVE_LABEL_H

#include "API.h"
#include "Widget.h"
#include <string>

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
        }
        NVE_API const std::string& GetText() const
        {
            return mText;
        }

        NVE_API void SetFont(const std::string& font)
        {
            mFont = font;
        }
        NVE_API const std::string& GetFont() const
        {
            return mFont;
        }

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

    private:
        std::string mText;
        std::string mFont;
    };
} // namespace nuvelocity

#endif // NVE_LABEL_H
