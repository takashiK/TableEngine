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
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QDir>
#include <QItemSelectionModel>
#include <QTemporaryDir>
#include <QTest>
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileSortProxyModel.h"

namespace {

void writeFile(const QString& path, int size)
{
	QFile file(path);
	ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
	ASSERT_EQ(file.write(QByteArray(size, 'x')), size);
}

QFileSystemModel* sourceModel(TeFileFolderView& view)
{
	auto* proxy = qobject_cast<TeFileSortProxyModel*>(view.list()->model());
	return proxy == nullptr ? nullptr : qobject_cast<QFileSystemModel*>(proxy->sourceModel());
}

QModelIndex proxyIndex(TeFileFolderView& view, const QString& path)
{
	auto* proxy = qobject_cast<TeFileSortProxyModel*>(view.list()->model());
	auto* model = sourceModel(view);
	return proxy == nullptr || model == nullptr ? QModelIndex() : proxy->mapFromSource(model->index(path));
}

QString currentPath(TeFileFolderView& view)
{
	auto* proxy = qobject_cast<TeFileSortProxyModel*>(view.list()->model());
	auto* model = sourceModel(view);
	return proxy == nullptr || model == nullptr
		? QString()
		: model->filePath(proxy->mapToSource(view.list()->currentIndex()));
}

QStringList selectedPaths(TeFileFolderView& view)
{
	QStringList paths;
	auto* proxy = qobject_cast<TeFileSortProxyModel*>(view.list()->model());
	auto* model = sourceModel(view);
	if (proxy == nullptr || model == nullptr)
		return paths;

	for (const QModelIndex& index : view.list()->selectionModel()->selectedIndexes())
		paths.append(model->filePath(proxy->mapToSource(index)));
	paths.sort();
	return paths;
}

// Exposes the protected collectFileContextMenuTarget() helper for direct,
// deterministic testing without invoking any native menu / QMenu::exec().
class TestableFileFolderView : public TeFileFolderView
{
public:
	using TeFileFolderView::collectFileContextMenuTarget;
};

}


TEST(tst_TeFileFolderView, refresh_reloads_size_and_restores_selection)
{
	QTemporaryDir directory;
	ASSERT_TRUE(directory.isValid());
	const QString firstPath = directory.filePath("first.bin");
	const QString secondPath = directory.filePath("second.bin");
	writeFile(firstPath, 10);
	writeFile(secondPath, 20);

	TeFileFolderView view;
	view.setRootPath(directory.path());
	auto* proxy = qobject_cast<TeFileSortProxyModel*>(view.list()->model());
	ASSERT_NE(proxy, nullptr);
	QTRY_VERIFY_WITH_TIMEOUT(proxyIndex(view, firstPath).isValid(), 5000);
	QTRY_VERIFY_WITH_TIMEOUT(proxyIndex(view, secondPath).isValid(), 5000);

	const QModelIndex firstIndex = proxyIndex(view, firstPath);
	const QModelIndex secondIndex = proxyIndex(view, secondPath);
	view.list()->selectionModel()->select(firstIndex, QItemSelectionModel::ClearAndSelect);
	view.list()->selectionModel()->select(secondIndex, QItemSelectionModel::Select);
	view.list()->setCurrentIndex(firstIndex);

	proxy->setSortType(TeTypes::ORDER_SIZE);
	proxy->sort(0, Qt::AscendingOrder);
	writeFile(firstPath, 30);

	QFileSystemModel* oldModel = sourceModel(view);
	view.refresh();
	QFileSystemModel* refreshedModel = sourceModel(view);
	ASSERT_NE(refreshedModel, nullptr);
	EXPECT_NE(refreshedModel, oldModel);
	QTRY_VERIFY_WITH_TIMEOUT(proxyIndex(view, firstPath).isValid(), 5000);
	QTRY_COMPARE_WITH_TIMEOUT(refreshedModel->fileInfo(refreshedModel->index(firstPath)).size(), qint64(30), 5000);
	QTRY_COMPARE_WITH_TIMEOUT(currentPath(view), firstPath, 5000);

	QStringList expectedPaths{firstPath, secondPath};
	expectedPaths.sort();
	QTRY_COMPARE_WITH_TIMEOUT(selectedPaths(view), expectedPaths, 5000);
	QTRY_COMPARE_WITH_TIMEOUT(refreshedModel->filePath(proxy->mapToSource(proxy->index(0, 0))), secondPath, 5000);
}

TEST(tst_TeFileFolderView, right_click_background_with_stale_selection_is_not_a_file_target)
{
	// Regression test: right-clicking the background while other files remain selected
	// from a prior selection must never surface that stale selection as a
	// native file context-menu target.
	QTemporaryDir directory;
	ASSERT_TRUE(directory.isValid());
	const QString firstPath = directory.filePath("first.bin");
	writeFile(firstPath, 10);

	TestableFileFolderView view;
	view.setRootPath(directory.path());
	QTRY_VERIFY_WITH_TIMEOUT(proxyIndex(view, firstPath).isValid(), 5000);

	const QModelIndex firstIndex = proxyIndex(view, firstPath);
	view.list()->selectionModel()->select(firstIndex, QItemSelectionModel::ClearAndSelect);

	QStringList files;
	const bool isItemTarget = view.collectFileContextMenuTarget(
		view.list(), QPoint(-1, -1), files);

	EXPECT_FALSE(isItemTarget);
	EXPECT_TRUE(files.isEmpty());
}

TEST(tst_TeFileFolderView, right_click_selected_item_is_a_file_target)
{
	// Baseline: right-clicking the current, selected item must still resolve
	// to a native file context-menu target.
	QTemporaryDir directory;
	ASSERT_TRUE(directory.isValid());
	const QString firstPath = directory.filePath("first.bin");
	writeFile(firstPath, 10);

	TestableFileFolderView view;
	view.setRootPath(directory.path());
	QTRY_VERIFY_WITH_TIMEOUT(proxyIndex(view, firstPath).isValid(), 5000);

	const QModelIndex firstIndex = proxyIndex(view, firstPath);
	view.list()->selectionModel()->select(firstIndex, QItemSelectionModel::ClearAndSelect);
	view.list()->setCurrentIndex(firstIndex);

	QStringList files;
	const bool isItemTarget = view.collectFileContextMenuTarget(
		view.list(), view.list()->visualRect(firstIndex).center(), files);

	EXPECT_TRUE(isItemTarget);
	ASSERT_EQ(1, files.size());
	EXPECT_EQ(QDir::cleanPath(firstPath), QDir::cleanPath(files.constFirst()));
}
TEST(tst_TeFileFolderView, invalid_root_keeps_previous_tree_list_and_history)
{
QTemporaryDir directory;
ASSERT_TRUE(directory.isValid());
const QString invalidPath = directory.filePath("missing/nested");

TeFileFolderView view;
ASSERT_TRUE(view.trySetRootPath(directory.path()));
const QString rootBefore = view.rootPath();
const QString currentBefore = view.currentPath();
const QStringList historyBefore = view.getPathHistory();
QFileSystemModel* model = sourceModel(view);
ASSERT_NE(model, nullptr);
const QString listRootBefore = model->rootPath();
const QModelIndex listRootIndexBefore = view.list()->rootIndex();

EXPECT_FALSE(view.trySetRootPath(invalidPath));

EXPECT_EQ(rootBefore, view.rootPath());
EXPECT_EQ(currentBefore, view.currentPath());
EXPECT_EQ(historyBefore, view.getPathHistory());
EXPECT_EQ(listRootBefore, model->rootPath());
EXPECT_EQ(listRootIndexBefore, view.list()->rootIndex());
EXPECT_EQ(QDir::cleanPath(directory.path()), QDir::cleanPath(view.rootPath()));
}

TEST(tst_TeFileFolderView, invalid_current_keeps_previous_state_and_valid_current_still_works)
{
QTemporaryDir directory;
ASSERT_TRUE(directory.isValid());
ASSERT_TRUE(QDir(directory.path()).mkpath("child"));
const QString childPath = directory.filePath("child");

TeFileFolderView view;
ASSERT_TRUE(view.trySetRootPath(directory.path()));
const QString currentBefore = view.currentPath();
const QStringList historyBefore = view.getPathHistory();
QFileSystemModel* model = sourceModel(view);
ASSERT_NE(model, nullptr);
const QString listRootBefore = model->rootPath();

EXPECT_FALSE(view.trySetCurrentPath(directory.filePath("missing")));
EXPECT_EQ(currentBefore, view.currentPath());
EXPECT_EQ(historyBefore, view.getPathHistory());
EXPECT_EQ(listRootBefore, model->rootPath());

EXPECT_TRUE(view.trySetCurrentPath(childPath));
EXPECT_EQ(QDir::cleanPath(childPath), QDir::cleanPath(view.currentPath()));
EXPECT_EQ(QDir::cleanPath(childPath), QDir::cleanPath(model->rootPath()));
}

TEST(tst_TeFileFolderView, unc_wsl_root_never_commits_an_invalid_index)
{
// Optional: only meaningful where a WSL UNC share exists (Windows); the guard must hold either way.
const QString wslPath = QStringLiteral("//wsl.localhost/Ubuntu");
if (!QFileInfo::exists(wslPath))
GTEST_SKIP() << "WSL UNC share is not available";

QTemporaryDir directory;
ASSERT_TRUE(directory.isValid());

TeFileFolderView view;
ASSERT_TRUE(view.trySetRootPath(directory.path()));
const QString rootBefore = view.rootPath();

if (view.trySetRootPath(wslPath)) {
EXPECT_FALSE(view.rootPath().isEmpty());
EXPECT_TRUE(view.list()->rootIndex().isValid());
}
else {
EXPECT_EQ(rootBefore, view.rootPath());
EXPECT_EQ(QDir::cleanPath(directory.path()), QDir::cleanPath(view.rootPath()));
}
}

TEST(tst_TeFileFolderView, try_set_paths_with_invalid_current_keeps_root_list_and_history)
{
	QTemporaryDir first;
	QTemporaryDir second;
	ASSERT_TRUE(first.isValid());
	ASSERT_TRUE(second.isValid());

	TeFileFolderView view;
	ASSERT_TRUE(view.trySetRootPath(first.path()));
	const QString rootBefore = view.rootPath();
	const QString currentBefore = view.currentPath();
	const QStringList historyBefore = view.getPathHistory();
	QFileSystemModel* model = sourceModel(view);
	ASSERT_NE(model, nullptr);
	const QString listRootBefore = model->rootPath();

	EXPECT_FALSE(view.trySetPaths(second.path(), second.filePath("missing")));

	EXPECT_EQ(rootBefore, view.rootPath());
	EXPECT_EQ(currentBefore, view.currentPath());
	EXPECT_EQ(historyBefore, view.getPathHistory());
	EXPECT_EQ(listRootBefore, model->rootPath());
}

TEST(tst_TeFileFolderView, try_set_paths_with_valid_root_and_current_commits_once)
{
	QTemporaryDir first;
	QTemporaryDir second;
	ASSERT_TRUE(first.isValid());
	ASSERT_TRUE(second.isValid());
	ASSERT_TRUE(QDir(second.path()).mkpath("child"));
	const QString childPath = second.filePath("child");

	TeFileFolderView view;
	ASSERT_TRUE(view.trySetRootPath(first.path()));
	const int historyBefore = view.getPathHistory().size();

	ASSERT_TRUE(view.trySetPaths(second.path(), childPath));

	EXPECT_EQ(QDir::cleanPath(second.path()), QDir::cleanPath(view.rootPath()));
	EXPECT_EQ(QDir::cleanPath(childPath), QDir::cleanPath(view.currentPath()));
	EXPECT_EQ(QDir::cleanPath(childPath), QDir::cleanPath(sourceModel(view)->rootPath()));
	EXPECT_EQ(historyBefore + 1, view.getPathHistory().size());
}

TEST(tst_TeFileFolderView, history_navigation_to_removed_path_does_not_advance_cursor)
{
	QTemporaryDir directory;
	ASSERT_TRUE(directory.isValid());
	ASSERT_TRUE(QDir(directory.path()).mkpath("a"));
	ASSERT_TRUE(QDir(directory.path()).mkpath("b"));
	const QString pathA = directory.filePath("a");
	const QString pathB = directory.filePath("b");

	TeFileFolderView view;
	ASSERT_TRUE(view.trySetRootPath(directory.path()));
	ASSERT_TRUE(view.trySetCurrentPath(pathA));
	ASSERT_TRUE(view.trySetCurrentPath(pathB));
	ASSERT_EQ(QDir::cleanPath(pathB), QDir::cleanPath(view.currentPath()));

	// Wait until the tree model notices the removal; skip if this platform never reports it.
	ASSERT_TRUE(QDir(directory.path()).rmdir("a"));
	auto* treeProxy = qobject_cast<TeFileSortProxyModel*>(view.tree()->model());
	auto* treeModel = treeProxy == nullptr ? nullptr : qobject_cast<QFileSystemModel*>(treeProxy->sourceModel());
	ASSERT_NE(treeModel, nullptr);
	for (int i = 0; i < 50 && treeModel->index(pathA).isValid(); ++i)
		QTest::qWait(100);
	if (treeModel->index(pathA).isValid())
		GTEST_SKIP() << "File system model did not report the removal";

	EXPECT_EQ(QDir::cleanPath(pathB), QDir::cleanPath(view.currentPath()));
	view.movePrevPath();
	EXPECT_EQ(QDir::cleanPath(pathB), QDir::cleanPath(view.currentPath()));
	view.movePrevPath();

	EXPECT_EQ(QDir::cleanPath(pathB), QDir::cleanPath(view.currentPath()));
}
