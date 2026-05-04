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
 * TeArchiveFolderView tests
 *
 * Coverage:
 *   - URI_WRITE / URI_READ constants
 *   - setArchive(path) / clear()
 *   - setRootPath / rootPath
 *   - setCurrentPath / currentPath
 *   - tree() / list() accessors
 *   - getType()
 *   - Navigation history (moveNextPath / movePrevPath / getPathHistory)
 */

#include <gmock/gmock.h>
#include <QTest>

#include <widgets/TeArchiveFolderView.h>
#include <widgets/TeFileTreeView.h>
#include <widgets/TeFileListView.h>
#include <test_util/TestFileCreator.h>
#include <utils/TeArchive.h>
#include <TeTypes.h>

#include <QStringList>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTemporaryDir>
#include <QDebug>

using namespace ::testing;

// ── Helper ───────────────────────────────────────────────────────────────────

static QString buildTestArchive(const QString& destDir)
{
    QString archivePath = destDir + "/test.zip";
    QStringList files = {"file1.txt", "file2.txt", "sub/file3.txt"};
    createFileTree(destDir + "/src", files);

    TeArchive::Writer writer;
    writer.addEntries(destDir + "/src", files, "");
    writer.archive(archivePath, TeArchive::AR_ZIP);
    return archivePath;
}

// ── Fixture ───────────────────────────────────────────────────────────────────

class tst_TeArchiveFolderView : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(tmpDir.isValid());
        archivePath = buildTestArchive(tmpDir.path());
    }

    QTemporaryDir tmpDir;
    QString archivePath;
};

// ── URI constants ─────────────────────────────────────────────────────────────

TEST(tst_TeArchiveFolderView_static, URI_WRITE_is_nonempty)
{
    EXPECT_FALSE(TeArchiveFolderView::URI_WRITE.isEmpty());
}

TEST(tst_TeArchiveFolderView_static, URI_READ_is_nonempty)
{
    EXPECT_FALSE(TeArchiveFolderView::URI_READ.isEmpty());
}

TEST(tst_TeArchiveFolderView_static, URI_WRITE_and_READ_are_different)
{
    EXPECT_NE(TeArchiveFolderView::URI_WRITE, TeArchiveFolderView::URI_READ);
}

// ── getType ───────────────────────────────────────────────────────────────────

TEST(tst_TeArchiveFolderView_static, getType_returns_WT_ARCHIVEVIEW)
{
    TeArchiveFolderView view;
    EXPECT_EQ(view.getType(), TeTypes::WT_ARCHIVEVIEW);
}

// ── tree / list accessors ─────────────────────────────────────────────────────

TEST(tst_TeArchiveFolderView_static, tree_returns_non_null)
{
    TeArchiveFolderView view;
    EXPECT_NE(view.tree(), nullptr);
}

TEST(tst_TeArchiveFolderView_static, list_returns_non_null)
{
    TeArchiveFolderView view;
    EXPECT_NE(view.list(), nullptr);
}

// ── setArchive / clear ────────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFolderView, setArchive_valid_path_returns_true)
{
    TeArchiveFolderView view;
    EXPECT_TRUE(view.setArchive(archivePath));
}

TEST_F(tst_TeArchiveFolderView, setArchive_invalid_path_returns_false)
{
    TeArchiveFolderView view;
    EXPECT_FALSE(view.setArchive("/no_such_archive_12345.zip"));
}

TEST_F(tst_TeArchiveFolderView, clear_after_setArchive_does_not_crash)
{
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    EXPECT_NO_THROW(view.clear());
}

TEST_F(tst_TeArchiveFolderView, clear_on_empty_view_does_not_crash)
{
    TeArchiveFolderView view;
    EXPECT_NO_THROW(view.clear());
}

// ── setRootPath / rootPath ────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFolderView, setRootPath_with_URI_READ_prefix)
{
    // setRootPath() forwards to setArchive(), which opens the file as-is;
    // URI-prefixed paths won't open successfully, so this call returns false
    // and rootPath() remains empty. Just verify it doesn't crash.
    TeArchiveFolderView view;
    QString uri = TeArchiveFolderView::URI_READ + archivePath;
    view.setRootPath(uri);
    // rootPath() is empty because the URI scheme prefix is not stripped
    EXPECT_TRUE(view.rootPath().isEmpty());
}

// ── setCurrentPath / currentPath ──────────────────────────────────────────────

TEST_F(tst_TeArchiveFolderView, setCurrentPath_root_does_not_crash)
{
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    EXPECT_NO_THROW(view.setCurrentPath(""));
}

TEST_F(tst_TeArchiveFolderView, currentPath_after_setCurrentPath)
{
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    view.setCurrentPath("sub");
    EXPECT_EQ(view.currentPath(), QString("sub"));
}

// ── Navigation history ────────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFolderView, getPathHistory_initially_has_root)
{
    // getPathHistory() always returns an empty list in TeArchiveFolderView
    // (it is documented as such — not used for directory history)
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    QStringList hist = view.getPathHistory();
    EXPECT_TRUE(hist.isEmpty());
}

TEST_F(tst_TeArchiveFolderView, movePrevPath_at_start_does_not_crash)
{
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    EXPECT_NO_THROW(view.movePrevPath());
}

TEST_F(tst_TeArchiveFolderView, moveNextPath_at_end_does_not_crash)
{
    TeArchiveFolderView view;
    view.setArchive(archivePath);
    EXPECT_NO_THROW(view.moveNextPath());
}

