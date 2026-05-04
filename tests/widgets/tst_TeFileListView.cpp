/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

/*
 * TeFileListView
 *
 * Table Engine
 * - Mouse
 *     Click    set current cell / clear selection 
 *     Double Click    activate / clear selection / set current cell
 *     Drag (select)    new region select
 *         
 * - Shift
 *     Click    selection (from prev current) / set current cell
 *     Double Click    activate / set current cell
 *     Drag (select)    append region select
 *         
 * - Ctrl
 *     Click    invert selection / set current cell
 *     Double Click    activate / set current cell
 *     Drag (select)    invert region select
 *
 * - Key        Move = (Up/Down/Left/Right)
 *     Move    move current cell / no effect to selection
 *     Enter    activate current cell
 *     space    invert selection of current cell / set current to incremeted position.
 *         
 * - Shift   
 *     Move    invert selection of current cell
 *     Enter    activate current cell
 *     space    invert selection of current cell / set current to decremented position.
 *         
 * - Ctrl    
 *     Move    move current cell / no effect to selection
 *     Enter    activate current cell
 *     space    invert selection of current cell / set current to incremeted position.
 *
 * Attention:
 *    - When setCurrentIndex() is called, selection is changed caused by before state. so test code should call setCurrentIndex() before clearSelection() if needed.
 */

#include <gmock/gmock.h>
#include <QTest>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>

#include "widgets/TeFileListView.h"

class tst_TeFileListView : public ::testing::Test {
public:
    tst_TeFileListView() {
        for (int i = 0; i < 25; i++) {
            list.append(new QStandardItem(QString::asprintf("item%02d", i)));
		}
    }

    ~tst_TeFileListView() {
        for (auto item : list) {
			delete item;
		}
        list.clear();
    }

protected:
    virtual void SetUp() {
        model.clear();
        model.invisibleRootItem()->appendRows(list);
    }

    void setupView(TeFileListView& view) {
		view.setModel(&model);
        view.setViewMode(QListView::ListMode);
        view.setWrapping(true);
        view.setResizeMode(QListView::Adjust);
        view.setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
        view.setSpacing(2);
        view.setSelectionRectVisible(true);
        view.setFont(QFont(QStringLiteral("Courier New"),9));
        view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // view matrix
        //  0  4  8 12 16 20 24
        //  1  5  9 13 17 21
        //  2  6 10 14 18 22
        //  3  7 11 15 19 23

        int itemH = view.sizeHintForRow(0);
        int itemW = view.sizeHintForColumn(0);
        int sp    = view.spacing();

        view.resize(7*(itemW + sp * 2) + view.frameWidth() * 2 + 5, 4 * (itemH + sp * 2) + view.frameWidth() * 2 + 5);

        view.setFileViewMode(TeTypes::FILEINFO_NONE, TeTypes::FILEVIEW_SMALL_ICON);
        view.setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
    }

    QModelIndex movedIndex(const TeFileListView& view , const QModelIndex& index, int key) {
        // QListView in ListMode uses TopToBottom flow.
        // moveCursor() for TopToBottom: Up/Down = row ±1 (sequential), Left/Right = geometric.
        int rowCount = view.model()->rowCount();
        switch (key) {
        case Qt::Key_Up:
            if (index.row() > 0)
                return view.model()->index(index.row() - 1, 0);
            return index;
        case Qt::Key_Down:
            if (index.row() < rowCount - 1)
                return view.model()->index(index.row() + 1, 0);
            return index;
        default:
            break;
        }
        // Left/Right: geometric navigation
        QRect rect = view.visualRect(index);
        switch (key) {
        case Qt::Key_Left:
            rect.translate(-rect.width(), 0);
            break;
        case Qt::Key_Right:
            rect.translate(rect.width(), 0);
            break;
        }
        QModelIndex moved = view.indexAt(rect.center());
        if (!moved.isValid())
            moved = index;
        return moved;
    }

    QPoint betweenPoint(const TeFileListView& view, int row1, int row2) {
        QModelIndex index1 = view.model()->index(row1, 0);
        QModelIndex index2 = view.model()->index(row2, 0);
        QRect rect1 = view.visualRect(index1);
        QRect rect2 = view.visualRect(index2);
        return rect1.center() + (rect2.center() - rect1.center()) / 2;
    }

    // virtual void TearDown() {}
    QList<QStandardItem*> list;
    QStandardItemModel model;
};


TEST_F(tst_TeFileListView, setCurrent)
{
    TeFileListView view;
    setupView(view);

    view.setCurrentIndex(model.index(0, 0));
    EXPECT_EQ(view.currentIndex(), model.index(0, 0));
    view.setCurrentIndex(model.index(1, 0));
    EXPECT_EQ(view.currentIndex(), model.index(1, 0));
}

TEST_F(tst_TeFileListView, key_move)
{
    //Move    move current cell / no effect to selection

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE( QTest::qWaitForWindowExposed(&view) );

    // current cell (0,0) is selected
    QModelIndex index = model.index(0, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Up);
    EXPECT_EQ(movedIndex(view,index,Qt::Key_Up), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Down);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Left);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Right);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    // current cell (8,0) / all items are selected
    index = model.index(8, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Up);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Down);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Left);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Right);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    // current cell (8,0) / no items are selected
    index = model.index(8, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());


    // current cell (24,0) / no item is selected
    index = model.index(0, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
}

TEST_F(tst_TeFileListView, key_enter)
{
    // Enter    activate current cell
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(0, 0);

    QObject::connect(&view, &TeFileListView::activated, [index](const QModelIndex& activeIndex) {
        EXPECT_EQ(index, activeIndex);
        });

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Enter); 
    QTest::keyClick(&view, Qt::Key_Return);

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Enter);
    QTest::keyClick(&view, Qt::Key_Return);

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1,0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Enter);
    QTest::keyClick(&view, Qt::Key_Return);

}

TEST_F(tst_TeFileListView, key_space)
{
    // space    invert selection of current cell / set current to incremeted position.
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    /////////////////////////////
    // top item
    QModelIndex index = view.model()->index(0, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(1,0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(1, 0), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // next cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(1, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    /////////////////////////////
    // bottom item
    index = view.model()->index(view.model()->rowCount()-1, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // prev cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(view.model()->rowCount() - 2, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(view.model()->rowCount() - 2, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, key_shift_move)
{
    // Shift Move    invert selection of current cell. move current cell

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    // current cell (0,0) is selected
    QModelIndex index = model.index(0, 0);

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Up, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0,view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Down, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // current cell (8,0) / all items are selected
    index = model.index(8, 0);

    view.setCurrentIndex(index);
    view.clearSelection();
    QModelIndex startIndex = model.index(0, 0);
    QModelIndex endIndex = model.index(model.rowCount() - 1, model.columnCount() - 1);
    QItemSelection selection(startIndex, endIndex);
    view.selectionModel()->select(selection, QItemSelectionModel::Select);

    QTest::keyClick(&view, Qt::Key_Up, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    //selection state of index is inverted, but all other items are still selected
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(movedIndex(view, index, Qt::Key_Up)));
    EXPECT_EQ(view.model()->rowCount()-1, view.selectionModel()->selectedRows().count());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(selection, QItemSelectionModel::Select);

    QTest::keyClick(&view, Qt::Key_Down, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount() - 1, view.selectionModel()->selectedRows().count());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(selection, QItemSelectionModel::Select);

    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount() - 1, view.selectionModel()->selectedRows().count());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(selection, QItemSelectionModel::Select);

    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount() - 1, view.selectionModel()->selectedRows().count());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    // current cell (8,0) / no items are selected
    index = model.index(8, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));


    // current cell (24,0) / no item is selected
    index = model.index(0, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, key_shift_enter)
{
    // Shift Enter    activate current cell
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(0, 0);

    QObject::connect(&view, &TeFileListView::activated, [index](const QModelIndex& activeIndex) {
        EXPECT_EQ(index, activeIndex);
        });

    view.clearSelection();
    view.setCurrentIndex(index);
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ShiftModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ShiftModifier);

    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    view.setCurrentIndex(index);
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ShiftModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ShiftModifier);

    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select);
    view.setCurrentIndex(index);
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ShiftModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ShiftModifier);

}

TEST_F(tst_TeFileListView, key_shift_space)
{
    // Shift space    invert selection of current cell / set current to decremented position.

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    /////////////////////////////
    // top item
    QModelIndex index = view.model()->index(0, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(0, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(0, 0), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // next cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(0, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    /////////////////////////////
    // bottom item
    index = view.model()->index(view.model()->rowCount() - 1, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(view.model()->rowCount() - 2, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(view.model()->rowCount() - 2, 0), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // prev cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(view.model()->rowCount() - 2, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ShiftModifier);
    EXPECT_EQ(view.model()->index(view.model()->rowCount() - 2, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(view.model()->rowCount() - 2, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

}

TEST_F(tst_TeFileListView, key_ctrl_move)
{
    // Ctrl Move    move current cell / no effect to selection

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    // current cell (0,0) is selected
    QModelIndex index = model.index(0, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Up, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Down, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Left, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Right, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));

    // current cell (8,0) / all items are selected
    index = model.index(8, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Up, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Down, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Left, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    QTest::keyClick(&view, Qt::Key_Right, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(view.model()->rowCount(), view.selectionModel()->selectedRows().count());

    // current cell (8,0) / no items are selected
    index = model.index(8, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());


    // current cell (24,0) / no item is selected
    index = model.index(0, 0);
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Up, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Up), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Down, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Down), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Left, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Left), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Right, Qt::ControlModifier);
    EXPECT_EQ(movedIndex(view, index, Qt::Key_Right), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
}

TEST_F(tst_TeFileListView, key_ctrl_enter)
{
    // Ctrl Enter    activate current cell

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(0, 0);

    QObject::connect(&view, &TeFileListView::activated, [index](const QModelIndex& activeIndex) {
        EXPECT_EQ(index, activeIndex);
        });

    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ControlModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ControlModifier);

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ControlModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ControlModifier);

    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Enter, Qt::ControlModifier);
    QTest::keyClick(&view, Qt::Key_Return, Qt::ControlModifier);
}

TEST_F(tst_TeFileListView, key_ctrl_space)
{
    // Ctrl space    invert selection of current cell / set current to incremeted position.

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    /////////////////////////////
    // top item
    QModelIndex index = view.model()->index(0, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(view.model()->index(1, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(view.model()->index(1, 0), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // next cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(view.model()->index(1, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    /////////////////////////////
    // bottom item
    index = view.model()->index(view.model()->rowCount() - 1, 0);

    // no selected item
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // current cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // prev cell is selected
    view.setCurrentIndex(index);
    view.clearSelection();
    view.selectionModel()->select(view.model()->index(view.model()->rowCount() - 2, 0), QItemSelectionModel::Select);
    QTest::keyClick(&view, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(view.model()->rowCount() - 2, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, mouse_click)
{
    // Click    set current cell / clear selection 
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);
    QRect gRect(view.mapToGlobal(rect.topLeft()),view.mapToGlobal(rect.bottomRight()));

    // no selected item click
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // space key after click selection 
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(9, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // click on selected item
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // space key after selected item click
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(9, 0), view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());

    // all item selected
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    // click middle of selected item index(8, 0) and index(9, 0)
    QRect rect2 = view.visualRect(view.model()->index(9, 0));
    QRect gRect2(view.mapToGlobal(rect2.topLeft()), view.mapToGlobal(rect2.bottomRight()));

    gRect |= gRect2;

    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, gRect.center());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, mouse_shift_click)
{
    // Shift Click    append selection (from prev current) / set current cell
        // Click    set current cell / clear selection 
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);

    // no selected item click (8,0) and click (10,0) with shift
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());

    index = view.model()->index(10, 0);
    rect = view.visualRect(index);

    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_EQ(view.model()->index(10, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_EQ(3, view.selectionModel()->selectedRows().count());

    // continue click (6,0) with shift
    index = view.model()->index(6, 0);
    rect = view.visualRect(index);

    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_EQ(view.model()->index(6, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(7, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_EQ(3, view.selectionModel()->selectedRows().count());


    // space key after click selection 
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(7, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(7, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_EQ(3, view.selectionModel()->selectedRows().count());

    // click on selected item
    index = view.model()->index(8, 0);
    rect = view.visualRect(index);

    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(7, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    // click middle of selected item index(8, 0) and index(9, 0)
    QPoint pos = betweenPoint(view,8,9);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, pos);
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    view.clearSelection();
    index = view.model()->index(8, 0);
    rect = view.visualRect(index);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    index = view.model()->index(13, 0);
    rect = view.visualRect(index);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(11, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(12, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(13, 0)));
    EXPECT_EQ(6, view.selectionModel()->selectedRows().count());

}

TEST_F(tst_TeFileListView, mouse_ctrl_click)
{
    // Ctrl Click    invert selection / set current cell

    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);
    QRect gRect(view.mapToGlobal(rect.topLeft()), view.mapToGlobal(rect.bottomRight()));

    // no selected item click
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // space key after click selection 
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(9, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // append selection by ctrl click
    index = view.model()->index(10, 0);
    rect = view.visualRect(index);
    gRect = QRect(view.mapToGlobal(rect.topLeft()), view.mapToGlobal(rect.bottomRight()));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    // click on selected item
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_FALSE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // space key after selected item click
    QTest::keyClick(&view, Qt::Key_Space);
    EXPECT_EQ(view.model()->index(11, 0), view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());

    // click middle of selected item index(8, 0) and index(9, 0)
    QRect rect2 = view.visualRect(view.model()->index(9, 0));
    QRect gRect2(view.mapToGlobal(rect2.topLeft()), view.mapToGlobal(rect2.bottomRight()));

    gRect |= gRect2;

    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, gRect.center());
    EXPECT_EQ(2, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, mouse_doubleclick)
{
    // Double Click    activate / clear selection / set current cell
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));


    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);
    QRect gRect(view.mapToGlobal(rect.topLeft()), view.mapToGlobal(rect.bottomRight()));

    QObject::connect(&view, &TeFileListView::activated, [&index](const QModelIndex& actIndex) { EXPECT_EQ(index, actIndex); });

    // no selected item click
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // click on selected item
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // all item selected
    for (int i = 0; i < model.rowCount(); i++) {
        view.selectionModel()->select(model.index(i, 0), QItemSelectionModel::Select);
    }
    // click middle of selected item index(8, 0) and index(9, 0)
    QRect rect2 = view.visualRect(view.model()->index(9, 0));
    QRect gRect2(view.mapToGlobal(rect2.topLeft()), view.mapToGlobal(rect2.bottomRight()));

    gRect |= gRect2;

    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, gRect.center());
    EXPECT_EQ(0, view.selectionModel()->selectedRows().count());
}

TEST_F(tst_TeFileListView, mouse_shift_doubleclick)
{
    // Shift Double Click    activate / set current cell
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));


    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);
    QRect gRect(view.mapToGlobal(rect.topLeft()), view.mapToGlobal(rect.bottomRight()));

    QObject::connect(&view, &TeFileListView::activated, [&index](const QModelIndex& actIndex) { EXPECT_EQ(index, actIndex); });

    // no selected item click
    view.setCurrentIndex(index);
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // click on selected item
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // click middle of selected item index(8, 0) and index(9, 0)
    QRect rect2 = view.visualRect(view.model()->index(9, 0));
    QRect gRect2(view.mapToGlobal(rect2.topLeft()), view.mapToGlobal(rect2.bottomRight()));

    gRect |= gRect2;

    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, gRect.center());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

}

TEST_F(tst_TeFileListView, mouse_ctrl_doubleclick)
{
    // Ctrl Double Click    activate / set current cell
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));


    QModelIndex index = view.model()->index(8, 0);
    QRect rect = view.visualRect(index);
    QRect gRect(view.mapToGlobal(rect.topLeft()), view.mapToGlobal(rect.bottomRight()));

    QObject::connect(&view, &TeFileListView::activated, [&index](const QModelIndex& actIndex) { EXPECT_EQ(index, actIndex); });

    // no selected item click
    view.setCurrentIndex(index);
    view.clearSelection();
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, rect.center());
    EXPECT_EQ(index, view.currentIndex());
    EXPECT_TRUE(view.selectionModel()->isSelected(index));
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

    // click middle of selected item index(8, 0) and index(9, 0)
    view.selectionModel()->select(index, QItemSelectionModel::Select);
    QRect rect2 = view.visualRect(view.model()->index(9, 0));
    QRect gRect2(view.mapToGlobal(rect2.topLeft()), view.mapToGlobal(rect2.bottomRight()));

    gRect |= gRect2;

    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, gRect.center());
    EXPECT_EQ(1, view.selectionModel()->selectedRows().count());

}

TEST_F(tst_TeFileListView, mouse_drag)
{
    // Drag (select)    new region select
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    // view matrix
    //  0  4  8 12 16 20 24
    //  1  5  9 13 17 21
    //  2  6 10 14 18 22
    //  3  7 11 15 19 23

        QPoint pos;

    //select vertial items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 6, 7));
    QTest::mouseMove(view.viewport(), betweenPoint(view, 9, 10));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 9, 10));
    EXPECT_EQ(4, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));

    //select horizontal items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 2, 6));
    QTest::mouseMove(view.viewport(), betweenPoint(view, 14, 18));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 14, 18));
    EXPECT_EQ(4, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(2, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(14, 0)));

    //select rectangel items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 0, 5));
    QTest::mouseMove(view.viewport(), betweenPoint(view, 14, 19));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, betweenPoint(view, 14, 19));
    EXPECT_EQ(12, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(0, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(4, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(12, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(13, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(2, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(14, 0)));

}

TEST_F(tst_TeFileListView, mouse_shift_drag)
{
    // Shift Drag (select)    append region select
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    // view matrix
    //  0  4  8 12 16 20 24
    //  1  5  9 13 17 21
    //  2  6 10 14 18 22
    //  3  7 11 15 19 23


    QPoint pos;

    //select vertial items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 6, 7));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 9, 10));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 9, 10));
    EXPECT_EQ(4, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));

    //select horizontal items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 2, 6));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 14, 18));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 14, 18));
    EXPECT_EQ(6, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));

    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(2, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(14, 0)));

    //select rectangel items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 0, 5));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 14, 19));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ShiftModifier, betweenPoint(view, 14, 19));
    EXPECT_EQ(12, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(9, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));

    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(2, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(14, 0)));

    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(0, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(4, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(12, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(13, 0)));

}


TEST_F(tst_TeFileListView, mouse_ctrl_drag)
{
    // Ctrl Drag (select)    invert region select
    TeFileListView view;
    setupView(view);
    view.show();
    EXPECT_TRUE(QTest::qWaitForWindowExposed(&view));

    // view matrix
    //  0  4  8 12 16 20 24
    //  1  5  9 13 17 21
    //  2  6 10 14 18 22
    //  3  7 11 15 19 23

    QPoint pos;

    //select vertial items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 4, 5));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 6, 7));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 6, 7));
    EXPECT_EQ(3, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(4, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));

    //select horizontal items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 2, 6));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 18, 22));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 18, 22));
    EXPECT_EQ(6, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(4, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(5, 0)));

    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(2, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(10, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(14, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(18, 0)));

    //select rectangel items by drag
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 0, 4));
    QTest::mouseEvent(QTest::MouseMove, view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 19, 23));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ControlModifier, betweenPoint(view, 19, 23));
    EXPECT_EQ(14, view.selectionModel()->selectedRows().count());
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(0, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(1, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(3, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(6, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(7, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(8, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(11, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(12, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(13, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(15, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(16, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(17, 0)));
    EXPECT_TRUE(view.selectionModel()->isSelected(view.model()->index(19, 0)));

}
