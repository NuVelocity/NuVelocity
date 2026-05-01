#ifndef NVE_UI_J_TAB_CONTROL_H
#define NVE_UI_J_TAB_CONTROL_H

#include "API.h"
#include "Widget.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nuvelocity
{
    class JTabControl : public Widget
    {
    public:
        NVE_API JTabControl();
        NVE_API virtual ~JTabControl() = default;

        NVE_API void AddTab(const std::string& caption);
        NVE_API void ClearTabs();

        NVE_API int GetSelectedIndex() const;
        NVE_API void SetSelectedIndex(int index);

        NVE_API const std::vector<std::string>& GetTabs() const;
        NVE_API void SetOnTabChanged(std::function<void(int)> callback);

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void AddChild(const std::shared_ptr<Widget>& widget);
        NVE_API const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

        NVE_API SDL_Point GetContentOrigin() const override;

        NVE_API int GetTabPadding() const
        {
            return mTabPadding;
        }
        NVE_API void SetTabPadding(int padding)
        {
            mTabPadding = padding;
        }

        NVE_API int GetTabHeaderHeight() const
        {
            return mTabHeaderHeight;
        }
        NVE_API void SetTabHeaderHeight(int height)
        {
            mTabHeaderHeight = height;
        }

    private:
        std::vector<std::string> mTabs;
        int mSelectedIndex = -1;
        int mTabPadding = 6;
        int mTabHeaderHeight = 24;
        std::function<void(int)> mOnTabChanged;

        // Internal layout helper
        struct TabRect
        {
            SDL_Rect rect;
            int index;
        };
        std::vector<TabRect> mTabRects;
        void UpdateTabRects(Game* game);

        std::vector<std::shared_ptr<Widget>> mChildren;
    };
} // namespace nuvelocity

#endif // NVE_UI_J_TAB_CONTROL_H
