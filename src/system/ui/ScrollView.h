#ifndef NVE_UI_SCROLL_VIEW_H
#define NVE_UI_SCROLL_VIEW_H

#include "API.h"
#include "Widget.h"
#include <memory>
#include <vector>

namespace nuvelocity
{
    class ScrollView : public Widget
    {
    public:
        NVE_API ScrollView();
        NVE_API virtual ~ScrollView() = default;

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void AddChild(const std::shared_ptr<Widget>& child);
        NVE_API const std::vector<std::shared_ptr<Widget>>& GetChildren() const
        {
            return mChildren;
        }

        NVE_API int GetScrollOffset() const
        {
            return mScrollOffset;
        }
        NVE_API int GetTotalContentHeight() const;

        NVE_API bool IsUpPressed() const
        {
            return mUpPressed;
        }
        NVE_API bool IsDownPressed() const
        {
            return mDownPressed;
        }

        NVE_API SDL_Point GetContentOrigin() const override;

    private:
        std::vector<std::shared_ptr<Widget>> mChildren;
        int mScrollOffset = 0;

        bool mDraggingThumb = false;
        int mDragStartY = 0;
        int mDragStartScroll = 0;

        bool mUpPressed = false;
        bool mDownPressed = false;
    };
} // namespace nuvelocity

#endif // NVE_UI_SCROLL_VIEW_H
