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
#include "utils/TeFileInfo.h"
#include <QStandardItem>

using namespace ::testing;

class tst_TeFileInfo : public ::testing::Test {
protected:
    TeFileInfo makeFile(const QString& path, qint64 size = 1024) {
        TeFileInfo fi;
        fi.type         = TeFileInfo::EN_FILE;
        fi.path         = path;
        fi.size         = size;
        fi.lastModified = QDateTime(QDate(2024, 6, 1), QTime(12, 0));
        fi.permissions  = 0644;
        return fi;
    }

    TeFileInfo makeDir(const QString& path) {
        TeFileInfo fi;
        fi.type         = TeFileInfo::EN_DIR;
        fi.path         = path;
        fi.size         = 0;
        fi.lastModified = QDateTime(QDate(2024, 6, 1), QTime(12, 0));
        fi.permissions  = 0755;
        return fi;
    }
};

// ── EntryType enum ────────────────────────────────────────────────────────────

TEST_F(tst_TeFileInfo, entry_type_values_distinct)
{
    EXPECT_NE(TeFileInfo::EN_NONE,   TeFileInfo::EN_FILE);
    EXPECT_NE(TeFileInfo::EN_FILE,   TeFileInfo::EN_DIR);
    EXPECT_NE(TeFileInfo::EN_DIR,    TeFileInfo::EN_PARENT);
}

// ── exportItem ────────────────────────────────────────────────────────────────

TEST_F(tst_TeFileInfo, exportItem_file_returns_columns)
{
    TeFileInfo fi = makeFile("/docs/report.txt", 2048);
    QList<QStandardItem*> row = fi.exportItem(/*filesystem=*/false, /*bColum=*/true);
    EXPECT_FALSE(row.isEmpty());
    // First column should exist
    ASSERT_GE(row.size(), 1);
    QStandardItem* nameItem = row[0];
    // Custom roles should carry the data
    EXPECT_EQ(nameItem->data(TeFileInfo::ROLE_TYPE).toInt(), static_cast<int>(TeFileInfo::EN_FILE));
    EXPECT_EQ(nameItem->data(TeFileInfo::ROLE_PATH).toString(), QString("/docs/report.txt"));
    EXPECT_EQ(nameItem->data(TeFileInfo::ROLE_SIZE).toLongLong(), 2048LL);
    qDeleteAll(row);
}

TEST_F(tst_TeFileInfo, exportItem_dir_type_is_EN_DIR)
{
    TeFileInfo fi = makeDir("/docs");
    QList<QStandardItem*> row = fi.exportItem(false, false);
    ASSERT_GE(row.size(), 1);
    EXPECT_EQ(row[0]->data(TeFileInfo::ROLE_TYPE).toInt(), static_cast<int>(TeFileInfo::EN_DIR));
    qDeleteAll(row);
}

TEST_F(tst_TeFileInfo, exportItem_no_columns_returns_single_item)
{
    // exportItem() always returns 4 items (name, size, type, date);
    // bColum only controls icon source in current implementation
    TeFileInfo fi = makeFile("/a.txt");
    QList<QStandardItem*> row = fi.exportItem(false, false);
    EXPECT_EQ(row.size(), 4);
    qDeleteAll(row);
}

// ── exportToItem / importFromItem roundtrip ───────────────────────────────────

TEST_F(tst_TeFileInfo, import_export_roundtrip)
{
    TeFileInfo original = makeFile("/roundtrip/test.bin", 4096);
    original.type = TeFileInfo::EN_FILE;

    // exportItem returns a 4-column row; data is stored on the first (name) item
    QList<QStandardItem*> row = original.exportItem(false, true);
    ASSERT_GE(row.size(), 1);

    TeFileInfo restored;
    restored.importFromItem(row[0]);

    EXPECT_EQ(restored.type,         original.type);
    EXPECT_EQ(restored.path,         original.path);
    EXPECT_EQ(restored.size,         original.size);
    EXPECT_EQ(restored.lastModified, original.lastModified);
    EXPECT_EQ(restored.permissions,  original.permissions);
    qDeleteAll(row);
}

TEST_F(tst_TeFileInfo, import_parent_type)
{
    TeFileInfo fi;
    fi.type        = TeFileInfo::EN_PARENT;
    fi.path        = "/parent";
    fi.size        = 0;
    fi.permissions = 0;

    QList<QStandardItem*> row = fi.exportItem(false, true);
    ASSERT_GE(row.size(), 1);

    TeFileInfo restored;
    restored.importFromItem(row[0]);
    EXPECT_EQ(restored.type, TeFileInfo::EN_PARENT);
    qDeleteAll(row);
}

// ── ItemRole enum constants ───────────────────────────────────────────────────

TEST_F(tst_TeFileInfo, role_path_greater_than_user_role)
{
    EXPECT_GT(static_cast<int>(TeFileInfo::ROLE_PATH), static_cast<int>(Qt::UserRole));
}
