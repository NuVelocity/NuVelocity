#ifndef NVE_UI_BUTTON_CONTAINER_H
#define NVE_UI_BUTTON_CONTAINER_H

#include "Insets.h"
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
        void Update(Game* game) override;
        void Draw(Game* game) override;

        void SetRect(const SDL_Rect& rect) override;

        void SetGap(int gap);
        int GetGap() const;

        void SetMargin(int margin);
        void SetMargin(int horizontal, int vertical);
        void SetMargin(int top, int right, int bottom, int left);
        const Insets& GetMargin() const;

        void SetAutoCenter(bool autoCenter);
        bool IsAutoCenter() const;

    protected:
        void DoLayout() override;

    private:
        std::vector<std::shared_ptr<Button>> mButtons;
        int mGap = 8;
        bool mAutoCenter = false;
    };
} // namespace nuvelocity

#endif // NVE_UI_BUTTON_CONTAINER_H
