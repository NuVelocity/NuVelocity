#ifndef NVE_UI_J_LIST_BOX_H
#define NVE_UI_J_LIST_BOX_H

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

        JListBox();
        virtual ~JListBox() = default;

        void AddColumn(const std::string& title, int width);
        void AddRow(const std::vector<std::string>& cells);
        void Clear();

        int GetSelectedIndex() const;
        void SetSelectedIndex(int index);

        void Update(Game* game) override;
        void Draw(Game* game) override;

        void SetRowHeight(int height)
        {
            mRowHeight = height;
        }
        void SetHeaderHeight(int height)
        {
            mHeaderHeight = height;
        }

        void SetOnSelectionChanged(std::function<void(int)> callback);

        const std::vector<Column>& GetColumns() const
        {
            return mColumns;
        }
        const std::vector<std::vector<std::string>>& GetRows() const
        {
            return mRows;
        }
        int GetHeaderHeight() const
        {
            return mHeaderHeight;
        }
        int GetRowHeight() const
        {
            return mRowHeight;
        }
        int GetScrollOffset() const
        {
            return mScrollOffset;
        }
        bool IsUpPressed() const
        {
            return mUpPressed;
        }
        bool IsDownPressed() const
        {
            return mDownPressed;
        }

        void SetColumnWidth(int index, int width);

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

        bool mUpPressed = false;
        bool mDownPressed = false;

        int mResizingColumn = -1;
        int mResizeStartX = 0;
        int mResizeStartWidth = 0;
    };
} // namespace nuvelocity

#endif // NVE_UI_J_LIST_BOX_H
