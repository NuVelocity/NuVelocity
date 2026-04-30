#ifndef NVE_UI_BUTTON_CONTAINER_H
#define NVE_UI_BUTTON_CONTAINER_H

#include "API.h"
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
        NVE_API ButtonContainer();

        NVE_API void AddButton(const std::shared_ptr<Button>& button);
        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void SetRect(const SDL_Rect& rect) override;

        NVE_API void SetGap(int gap);
        NVE_API int GetGap() const;

        NVE_API void SetMargin(int margin);
        NVE_API void SetMargin(int horizontal, int vertical);
        NVE_API void SetMargin(int top, int right, int bottom, int left);
        NVE_API const Insets& GetMargin() const;

        NVE_API void SetAutoCenter(bool autoCenter);
        NVE_API bool IsAutoCenter() const;

    protected:
        NVE_API void DoLayout() override;

    private:
        std::vector<std::shared_ptr<Button>> mButtons;
        int mGap = 8;
        bool mAutoCenter = false;
    };
} // namespace nuvelocity

#endif // NVE_UI_BUTTON_CONTAINER_H
