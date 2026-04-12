#ifndef NVE_UI_SKINNED_BUTTON_H
#define NVE_UI_SKINNED_BUTTON_H

#include "Button.h"

#include <Sequence.h>

namespace nuvelocity
{
    class SkinnedButton : public Button
    {
    public:
        struct Skin
        {
            Sequence* normal = nullptr;
            Sequence* hover = nullptr;
            Sequence* pressed = nullptr;
            Sequence* disabled = nullptr;
        };

        SkinnedButton();

        void Draw(Game* game) override;

        void SetSkin(const Skin& skin);
        const Skin& GetSkin() const;

    private:
        Skin mSkin;
    };
} // namespace nuvelocity

#endif // NVE_UI_SKINNED_BUTTON_H
