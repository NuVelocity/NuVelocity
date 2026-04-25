#ifndef NVE_UI_BUTTON_CONTAINER_H
#define NVE_UI_BUTTON_CONTAINER_H

#include "Widget.h"
#include <memory>
#include <vector>

namespace nuvelocity
{
    class Button;

    class ButtonContainer : public Widget
    {
    public:
        ButtonContainer();

        void AddButton(const std::shared_ptr<Button>& button);
        void Update(Game* aGame) override;
        void Draw(Game* aGame) override;

        void SetSpacing(int spacing);
        int GetSpacing() const;

        struct Padding
        {
            int top = 0;
            int right = 0;
            int bottom = 0;
            int left = 0;
        };

        void SetPadding(int padding);
        void SetPadding(int horizontal, int vertical);
        void SetPadding(int top, int right, int bottom, int left);
        const Padding& GetPadding() const;

        void SetAutoCenter(bool autoCenter);
        bool IsAutoCenter() const;

    private:
        std::vector<std::shared_ptr<Button>> mButtons;
        int mSpacing = 8;
        Padding mPadding;
        bool mAutoCenter = false;
    };
} // namespace nuvelocity

#endif // NVE_UI_BUTTON_CONTAINER_H
