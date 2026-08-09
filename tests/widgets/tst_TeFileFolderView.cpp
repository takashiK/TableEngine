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
#include <QItemSelectionModel>
#include <QTemporaryDir>
#include <QTest>
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"
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