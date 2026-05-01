#ifndef NVE_UI_J_LIST_BOX_H
#define NVE_UI_J_LIST_BOX_H

#include "API.h"
#include "Widget.h"
#include <functional>
#include <string>
#include <vector>

namespace nuvelocity
{
    class JListBox : public Widget
    {
    public:
        struct Column
        {
            std::string title;
            int width;
        };

        NVE_API JListBox();
        NVE_API virtual ~JListBox() = default;

        NVE_API void AddColumn(const std::string& title, int width);
        NVE_API void AddRow(const std::vector<std::string>& cells);
        NVE_API void Clear();

        NVE_API int GetSelectedIndex() const;
        NVE_API void SetSelectedIndex(int index);

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void SetRowHeight(int height)
        {
            mRowHeight = height;
        }
        NVE_API void SetHeaderHeight(int height)
        {
            mHeaderHeight = height;
        }

        NVE_API void SetOnSelectionChanged(std::function<void(int)> callback);

        NVE_API const std::vector<Column>& GetColumns() const
        {
            return mColumns;
        }
        NVE_API const std::vector<std::vector<std::string>>& GetRows() const
        {
            return mRows;
        }
        NVE_API int GetHeaderHeight() const
        {
            return mHeaderHeight;
        }
        NVE_API int GetRowHeight() const
        {
            return mRowHeight;
        }
        NVE_API int GetScrollOffset() const
        {
            return mScrollOffset;
        }
        NVE_API bool IsUpPressed() const
        {
            return mUpPressed;
        }
        NVE_API bool IsDownPressed() const
        {
            return mDownPressed;
        }

        NVE_API void SetColumnWidth(int index, int width);

    private:
        std::vector<Column> mColumns;
        std::vector<std::vector<std::string>> mRows;
        int mSelectedIndex = -1;
        int mScrollOffset = 0;
        int mRowHeight = 15;
        int mHeaderHeight = 15;
        std::function<void(int)> mOnSelectionChanged;

        uint64_t mLastNavTick = 0;
        SDL_Scancode mLastNavCode = SDL_SCANCODE_UNKNOWN;

        bool mDraggingThumb = false;
        int mDragStartY = 0;
        int mDragStartScroll = 0;
        bool mSelectingDrag = false;

        bool mUpPressed = false;
        bool mDownPressed = false;

        int mResizingColumn = -1;
        int mResizeStartX = 0;
        int mResizeStartWidth = 0;
    };
} // namespace nuvelocity

#endif // NVE_UI_J_LIST_BOX_H
