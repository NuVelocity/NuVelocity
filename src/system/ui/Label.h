#ifndef NVE_LABEL_H
#define NVE_LABEL_H

#include "Widget.h"
#include <string>

namespace nuvelocity
{
    class Label : public Widget
    {
    public:
        Label(const std::string& text = "", const std::string& font = "OCR");
        virtual ~Label() = default;

        void SetText(const std::string& text) { mText = text; }
        const std::string& GetText() const { return mText; }

        void SetFont(const std::string& font) { mFont = font; }
        const std::string& GetFont() const { return mFont; }

        void Update(Game* aGame) override;
        void Draw(Game* aGame) override;

    private:
        std::string mText;
        std::string mFont;
    };
} // namespace nuvelocity

#endif // NVE_LABEL_H
