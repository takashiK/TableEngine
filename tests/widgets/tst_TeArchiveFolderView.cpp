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
#include <QAbstractProxyModel>
#include <QApplication>
#include <QPointer>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>

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

TEST_F(tst_TeArchiveFolderView, setArchive_processes_gui_events_before_returning)
{
    const QString singlePath = tmpDir.path() + "/single.zip";
    TeArchive::Writer writer;
    ASSERT_TRUE(writer.addEntry(tmpDir.path() + "/src/file1.txt", "only.txt"));
    ASSERT_TRUE(writer.archive(singlePath, TeArchive::AR_ZIP));
    TeArchiveFolderView view;
    bool callbackRan = false;
    QTimer::singleShot(0, &view, [&]() {
        callbackRan = true;
        EXPECT_EQ(QThread::currentThread(), qApp->thread());
        bool foundProgress = false;
        for (QWidget* window : QApplication::topLevelWidgets()) {
            if (auto* progress = qobject_cast<QProgressDialog*>(window)) {
                foundProgress = true;
                EXPECT_EQ(progress->minimumDuration(), 500);
                EXPECT_EQ(progress->minimum(), 0);
                EXPECT_EQ(progress->maximum(), 0);
                EXPECT_EQ(progress->value(), 0);
                EXPECT_FALSE(progress->isVisible());
                EXPECT_EQ(progress->windowModality(), Qt::ApplicationModal);
            }
        }
        EXPECT_TRUE(foundProgress);
    });

    ASSERT_TRUE(view.setArchive(singlePath));
    EXPECT_TRUE(callbackRan);
    EXPECT_EQ(view.currentFileInfo().path, QString("only.txt"));
    ASSERT_NE(view.reader(), nullptr);
    EXPECT_EQ(view.reader()->thread(), qApp->thread());
}

TEST_F(tst_TeArchiveFolderView, setArchive_batches_model_updates_on_gui_thread)
{
    const QString manyPath = tmpDir.path() + "/many.zip";
    TeArchive::Writer writer;
    const int entryCount = 300;
    for (int index = 0; index < entryCount; ++index) {
        ASSERT_TRUE(writer.addEntry(tmpDir.path() + "/src/file1.txt",
            QString("file%1.txt").arg(index, 4, 10, QChar('0'))));
    }
    ASSERT_TRUE(writer.archive(manyPath, TeArchive::AR_ZIP));
    TeArchiveFolderView view;
    auto* proxy = qobject_cast<QAbstractProxyModel*>(view.list()->model());
    ASSERT_NE(proxy, nullptr);
    auto* model = proxy->sourceModel();
    bool scheduled = false;
    bool callbackRan = false;
    QObject::connect(model, &QAbstractItemModel::rowsInserted, &view,
        [&](const QModelIndex& parent, int, int) {
            EXPECT_EQ(QThread::currentThread(), qApp->thread());
            if (parent.isValid() && !scheduled) {
                scheduled = true;
                QTimer::singleShot(0, &view, [&]() {
                    callbackRan = true;
                    EXPECT_GT(model->rowCount(model->index(0, 0)), 0);
                    EXPECT_LT(model->rowCount(model->index(0, 0)), entryCount);
                });
            }
        });

    ASSERT_TRUE(view.setArchive(manyPath));
    EXPECT_TRUE(callbackRan);
    EXPECT_EQ(model->rowCount(model->index(0, 0)), entryCount);
    EXPECT_EQ(view.currentFileInfo().path, QString("file0000.txt"));
}

TEST_F(tst_TeArchiveFolderView, setArchive_rejects_reentry_and_restores_window_input)
{
    QWidget window;
    window.show();
    TeArchiveFolderView view;
    bool callbackRan = false;
    QTimer::singleShot(0, &view, [&]() {
        callbackRan = true;
        EXPECT_FALSE(window.isEnabled());
        EXPECT_FALSE(view.setArchive(archivePath));
        view.setRootPath(TeArchiveFolderView::URI_WRITE + "ignored.zip");
        EXPECT_FALSE(view.newArchive(tmpDir.path() + "/ignored.zip"));
    });

    ASSERT_TRUE(view.setArchive(archivePath));
    EXPECT_TRUE(callbackRan);
    EXPECT_TRUE(window.isEnabled());
    EXPECT_TRUE(view.isReadOnly());
    EXPECT_EQ(view.entryInfo({"file1.txt", "file2.txt", "sub/file3.txt"}).size(), 3);
}

TEST_F(tst_TeArchiveFolderView, clear_during_setArchive_cancels_without_taking_reader)
{
    TeArchive::Reader reader;
    ASSERT_TRUE(reader.open(archivePath));
    TeArchiveFolderView view;
    QTimer::singleShot(0, &view, [&]() { view.clear(); });

    EXPECT_FALSE(view.setArchive(&reader));
    EXPECT_EQ(view.reader(), nullptr);
    EXPECT_TRUE(view.rootPath().isEmpty());
    EXPECT_TRUE(reader.isValid());
    EXPECT_TRUE(view.setArchive(archivePath));
}

TEST_F(tst_TeArchiveFolderView, destruction_during_setArchive_retains_caller_reader)
{
    TeArchive::Reader reader;
    ASSERT_TRUE(reader.open(archivePath));
    auto* view = new TeArchiveFolderView;
    QPointer<TeArchiveFolderView> guard(view);
    QTimer::singleShot(0, view, [view]() { delete view; });

    EXPECT_FALSE(view->setArchive(&reader));
    EXPECT_TRUE(guard.isNull());
    EXPECT_TRUE(reader.isValid());
}

TEST_F(tst_TeArchiveFolderView, destruction_during_path_setArchive_is_safe)
{
    auto* view = new TeArchiveFolderView;
    QPointer<TeArchiveFolderView> guard(view);
    QTimer::singleShot(0, view, [view]() { delete view; });

    EXPECT_FALSE(view->setArchive(archivePath));
    EXPECT_TRUE(guard.isNull());
}

TEST_F(tst_TeArchiveFolderView, setArchive_success_owns_reader_until_clear)
{
    auto* reader = new TeArchive::Reader;
    QPointer<TeArchive::Reader> guard(reader);
    ASSERT_TRUE(reader->open(archivePath));
    TeArchiveFolderView view;

    ASSERT_TRUE(view.setArchive(reader));
    EXPECT_EQ(view.reader(), reader);
    EXPECT_FALSE(view.setArchive(reader));
    view.clear();
    EXPECT_TRUE(guard.isNull());
    EXPECT_EQ(view.reader(), nullptr);
}

TEST_F(tst_TeArchiveFolderView, worker_open_failure_retains_caller_reader)
{
    TeArchive::Reader reader;
    ASSERT_TRUE(reader.open(archivePath));
    ASSERT_TRUE(QFile::remove(archivePath));
    TeArchiveFolderView view;

    EXPECT_FALSE(view.setArchive(&reader));
    EXPECT_EQ(view.reader(), nullptr);
    EXPECT_TRUE(view.rootPath().isEmpty());
    EXPECT_TRUE(reader.isValid());
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
    // setRootPath() strips the URI_READ scheme prefix and opens the archive in
    // read-only mode, so rootPath() resolves to the underlying archive path.
    TeArchiveFolderView view;
    QString uri = TeArchiveFolderView::URI_READ + archivePath;
    view.setRootPath(uri);
    EXPECT_TRUE(view.isReadOnly());
    EXPECT_FALSE(view.rootPath().isEmpty());
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

