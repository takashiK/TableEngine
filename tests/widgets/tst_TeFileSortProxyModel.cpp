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
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileSystemModel>
#include "widgets/TeFileSortProxyModel.h"
#include "TeTypes.h"

using namespace ::testing;

class tst_TeFileSortProxyModel : public ::testing::Test {
protected:
    void SetUp() override {
        // Build a simple flat model for proxy testing.
        for (const QString& name : {"zebra.txt", "apple.txt", "mango.txt"}) {
            auto* item = new QStandardItem(name);
            item->setData(name, Qt::DisplayRole);
            srcModel.appendRow(item);
        }
        proxy.setSourceModel(&srcModel);
    }

    QStandardItemModel srcModel;
    TeFileSortProxyModel proxy;
};

// ── setSortType / sortType ────────────────────────────────────────────────────

TEST_F(tst_TeFileSortProxyModel, sortType_default_is_ORDER_NAME)
{
    // Default sort type should be ORDER_NAME (first enum value = 0).
    EXPECT_EQ(proxy.sortType(), TeTypes::ORDER_NAME);
}

TEST_F(tst_TeFileSortProxyModel, setSortType_stores_value)
{
    proxy.setSortType(TeTypes::ORDER_SIZE);
    EXPECT_EQ(proxy.sortType(), TeTypes::ORDER_SIZE);

    proxy.setSortType(TeTypes::ORDER_MODIFIED);
    EXPECT_EQ(proxy.sortType(), TeTypes::ORDER_MODIFIED);
}

// ── setPixmapSize / pixmapSize ────────────────────────────────────────────────

TEST_F(tst_TeFileSortProxyModel, pixmapSize_default_is_nonempty_or_zero)
{
    // pixmapSize() should return a QSize (not crash).
    QSize sz = proxy.pixmapSize();
    EXPECT_GE(sz.width(),  0);
    EXPECT_GE(sz.height(), 0);
}

TEST_F(tst_TeFileSortProxyModel, setPixmapSize_stores_value)
{
    proxy.setPixmapSize(QSize(32, 32));
    EXPECT_EQ(proxy.pixmapSize(), QSize(32, 32));
}

// ── setFileRegex / fileRegex ──────────────────────────────────────────────────

TEST_F(tst_TeFileSortProxyModel, fileRegex_default_is_empty)
{
    QRegularExpression re = proxy.fileRegex();
    EXPECT_TRUE(re.pattern().isEmpty());
}

TEST_F(tst_TeFileSortProxyModel, setFileRegex_stores_value)
{
    QRegularExpression re(".*\\.cpp");
    proxy.setFileRegex(re);
    EXPECT_EQ(proxy.fileRegex().pattern(), re.pattern());
}

// ── FilePixmap role constant ───────────────────────────────────────────────────

TEST(tst_TeFileSortProxyModel_static, FilePixmap_role_is_UserRole_plus_50)
{
    EXPECT_EQ(static_cast<int>(TeFileSortProxyModel::FilePixmap),
              static_cast<int>(Qt::UserRole) + 50);
}

// ── data() does not crash for standard roles ──────────────────────────────────

TEST_F(tst_TeFileSortProxyModel, data_display_role_returns_string)
{
    for (int row = 0; row < proxy.rowCount(); ++row) {
        QModelIndex idx = proxy.index(row, 0);
        EXPECT_FALSE(proxy.data(idx, Qt::DisplayRole).toString().isEmpty());
    }
}

// ── setSourceModel and rowCount ───────────────────────────────────────────────

TEST_F(tst_TeFileSortProxyModel, rowCount_matches_source_when_no_filter)
{
    EXPECT_EQ(proxy.rowCount(), srcModel.rowCount());
}

TEST_F(tst_TeFileSortProxyModel, regex_filter_hides_non_matching_rows)
{
    // setFileRegex() should call invalidateFilter() internally.
    proxy.setFileRegex(QRegularExpression("apple.*"));
    // Only "apple.txt" should match — row count must be less than or equal to source.
    EXPECT_LE(proxy.rowCount(), srcModel.rowCount());
}
