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

#include <gmock/gmock.h>
#include "TeTypes.h"

using namespace ::testing;

// ── WidgetType ────────────────────────────────────────────────────────────────

TEST(tst_TeTypes, widget_type_none_is_zero)
{
    EXPECT_EQ(static_cast<int>(TeTypes::WT_NONE), 0);
}

TEST(tst_TeTypes, widget_type_values_distinct)
{
    EXPECT_NE(TeTypes::WT_NONE,       TeTypes::WT_MAINWINDOW);
    EXPECT_NE(TeTypes::WT_MAINWINDOW, TeTypes::WT_LISTVIEW);
    EXPECT_NE(TeTypes::WT_LISTVIEW,   TeTypes::WT_TREEVIEW);
    EXPECT_NE(TeTypes::WT_TREEVIEW,   TeTypes::WT_ARCHIVEVIEW);
}

// ── SelectionMode ─────────────────────────────────────────────────────────────

TEST(tst_TeTypes, selection_mode_none_is_first)
{
    EXPECT_EQ(static_cast<int>(TeTypes::SELECTION_NONE), 0);
}

TEST(tst_TeTypes, selection_mode_values_distinct)
{
    EXPECT_NE(TeTypes::SELECTION_NONE,         TeTypes::SELECTION_EXPLORER);
    EXPECT_NE(TeTypes::SELECTION_EXPLORER,     TeTypes::SELECTION_TABLE_ENGINE);
}

// ── FileViewMode ──────────────────────────────────────────────────────────────

TEST(tst_TeTypes, file_view_mode_values_distinct)
{
    EXPECT_NE(TeTypes::FILEVIEW_SMALL_ICON,  TeTypes::FILEVIEW_LARGE_ICON);
    EXPECT_NE(TeTypes::FILEVIEW_LARGE_ICON,  TeTypes::FILEVIEW_HUGE_ICON);
    EXPECT_NE(TeTypes::FILEVIEW_HUGE_ICON,   TeTypes::FILEVIEW_DETAIL_LIST);
}

// ── OrderType ─────────────────────────────────────────────────────────────────

TEST(tst_TeTypes, order_type_values_distinct)
{
    EXPECT_NE(TeTypes::ORDER_NAME,      TeTypes::ORDER_SIZE);
    EXPECT_NE(TeTypes::ORDER_SIZE,      TeTypes::ORDER_EXTENSION);
    EXPECT_NE(TeTypes::ORDER_EXTENSION, TeTypes::ORDER_MODIFIED);
}

// ── FileInfo flags ────────────────────────────────────────────────────────────

TEST(tst_TeTypes, fileinfo_none_is_zero)
{
    EXPECT_EQ(static_cast<int>(TeTypes::FILEINFO_NONE), 0);
}

TEST(tst_TeTypes, fileinfo_all_includes_size_and_modified)
{
    int allFlags = static_cast<int>(TeTypes::FILEINFO_ALL);
    EXPECT_NE(allFlags & static_cast<int>(TeTypes::FILEINFO_SIZE), 0);
    EXPECT_NE(allFlags & static_cast<int>(TeTypes::FILEINFO_MODIFIED), 0);
}

// ── FileType flags ────────────────────────────────────────────────────────────

TEST(tst_TeTypes, filetype_none_is_zero)
{
    EXPECT_EQ(static_cast<int>(TeTypes::FILETYPE_NONE), 0);
}

TEST(tst_TeTypes, filetype_all_includes_hidden_and_system)
{
    int allFlags = static_cast<int>(TeTypes::FILETYPE_ALL);
    EXPECT_NE(allFlags & static_cast<int>(TeTypes::FILETYPE_HIDDEN), 0);
    EXPECT_NE(allFlags & static_cast<int>(TeTypes::FILETYPE_SYSTEM), 0);
}

// ── CmdId ─────────────────────────────────────────────────────────────────────

TEST(tst_TeTypes, cmd_id_none_is_zero)
{
    EXPECT_EQ(static_cast<int>(TeTypes::CMDID_NONE), 0);
}

TEST(tst_TeTypes, cmd_id_system_groups_are_ordered)
{
    EXPECT_LT(static_cast<int>(TeTypes::CMDID_SYSTEM_FILE),
              static_cast<int>(TeTypes::CMDID_SYSTEM_FILE_NEW));
}
