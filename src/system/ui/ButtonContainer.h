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

        void SetSpacing(float spacing);
        float GetSpacing() const;

        struct Padding
        {
            float top = 0.0f;
            float right = 0.0f;
            float bottom = 0.0f;
            float left = 0.0f;
        };

        void SetPadding(float padding);
        void SetPadding(float horizontal, float vertical);
        void SetPadding(float top, float right, float bottom, float left);
        const Padding& GetPadding() const;

        void SetAutoCenter(bool autoCenter);
        bool IsAutoCenter() const;

    private:
        std::vector<std::shared_ptr<Button>> mButtons;
        float mSpacing = 8.0f;
        Padding mPadding;
        bool mAutoCenter = false;
    };
}

#endif // NVE_UI_BUTTON_CONTAINER_H
