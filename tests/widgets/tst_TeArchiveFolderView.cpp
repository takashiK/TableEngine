/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <gmock/gmock.h>

#include <widgets/TeArchiveFolderView.h>
#include <widgets/TeFileTreeView.h>
#include <widgets/TeFileListView.h>
#include <test_util/FileEntry.h>
#include <test_util/TestFileCreator.h>

#include <QStringList>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDebug>

QString modelPath(const QStandardItem* item) 
{
	QString path;

	while (item != nullptr) {
		path.prepend("/" + item->text());
		item = item->parent();
	}

	return path;
}

bool checkEntries(const FileEntry* expect, const QStandardItem* check) 
{
	bool result = true;
	for (int i = 0; i < check->rowCount(); i++) {
		const FileEntry* child = expect->findChild<FileEntry*>(check->child(i)->data(Qt::DisplayRole).toString());
		EXPECT_NE(nullptr, child) << "Illegal entry: " << modelPath(check->child(i)).toStdString() << " CheckParent: " << expect->objectName().toStdString();
		if ( child == nullptr) {
			result = false;
		}
		else {
			if (check->child(i, TeArchiveFolderView::COL_TYPE)->data().toInt() == TeArchive::EN_FILE) {
//				qDebug() << check->child(i)->data(Qt::DisplayRole).toString();
				EXPECT_EQ(child->size, check->child(i, TeArchiveFolderView::COL_SIZE)->data().toInt());
				EXPECT_EQ(child->lastModifyed, check->child(i, TeArchiveFolderView::COL_DATE)->data().toDateTime());
			}
			if (check->child(i)->hasChildren()) {
//				qDebug() << "Sub: " << child->path << " : " << check->child(i)->data(Qt::DisplayRole).toString();
				result = checkEntries(child, check->child(i));
			}
		}
	}

	for (auto entry : expect->children()) {
		int i = 0;
		for (i = 0; i < check->rowCount(); i++) {
			if (qobject_cast<FileEntry*>(entry)->path == check->child(i)->data(Qt::DisplayRole).toString()) {
				break;
			}
		}
		if (i >= check->rowCount()) {
			ADD_FAILURE() << "Failed Added: " << qobject_cast<FileEntry*>(entry)->path.toStdString();
			result = false;
		}
	}
	return result;
}

TEST(tst_TeArchiveFolderView, addDirEntry_normal)
{
	//Parent directories are added before children.
	QStringList paths;
	paths.append("dir1");
	paths.append("dir2");
	paths.append("dir3");
	paths.append("dir1/dir1_1");
	paths.append("dir1/dir1_2");
	paths.append("dir3/dir3_1");
	paths.append("dir3/dir3_2");
	paths.append("dir3/dir3_3");
	paths.append("dir1/dir1_1/dir1_1_1");
	paths.append("dir3/dir3_2/dir3_2_1");
	paths.append("dir3/dir3_2/dir3_2_2");

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, paths);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		folderView.addDirEntry(path);
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}

TEST(tst_TeArchiveFolderView, addDirEntry_shuffleAdd)
{
	//orders of adding are shuffle.
	QStringList paths;
	paths.append("dir3/dir3_2/dir3_2_1");
	paths.append("dir1/dir1_1");
	paths.append("dir1/dir1_2");
	paths.append("dir1");
	paths.append("dir3/dir3_1");
	paths.append("dir2");
	paths.append("dir3/dir3_2");
	paths.append("dir3");
	paths.append("dir3/dir3_3");
	paths.append("dir1/dir1_1/dir1_1_1");
	paths.append("dir3/dir3_2/dir3_2_2");

	FileEntry entries(nullptr,"root");
	expectEntries(&entries, paths);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		folderView.addDirEntry(path);
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}

TEST(tst_TeArchiveFolderView, addDirEntry_skipAdd)
{
	//directry add deep directory
	QStringList paths;
	paths.append("dir3/dir3_2/dir3_2_1");
	paths.append("dir1/dir1_1/dir1_1_1");
	paths.append("dir3/dir3_2/dir3_2_2");

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, paths);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		folderView.addDirEntry(path);
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}

TEST(tst_TeArchiveFolderView, addFileEntry_normal)
{
	//Parent directories are added before children.
	QStringList paths;
	paths.append("dir1");
	paths.append("dir2");
	paths.append("dir3");
	paths.append("dir1/dir1_1");
	paths.append("dir1/dir1_2");
	paths.append("dir1/file1_1.txt");
	paths.append("dir1/file1_2.txt");
	paths.append("dir3/dir3_1");
	paths.append("dir3/dir3_2");
	paths.append("dir3/dir3_2/file3_2_1.txt");
	paths.append("dir3/dir3_3");
	paths.append("dir1/dir1_1/dir1_1_1");
	paths.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	paths.append("dir3/dir3_2/dir3_2_1");
	paths.append("dir3/dir3_2/dir3_2_2");

	QDateTime date(QDateTime(QDate(2019, 3, 31)));

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, paths, date);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		if (path.indexOf(".") > 0) {
			folderView.addEntry(path, path.size(), date, path);
		}
		else {
			folderView.addDirEntry(path);
		}
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}

TEST(tst_TeArchiveFolderView, addFileEntry_shuffle)
{
	//orders of adding are shuffle.
	QStringList paths;
	paths.append("dir1/file1_2.txt");
	paths.append("dir1");
	paths.append("dir1/file1_1.txt");
	paths.append("dir1/dir1_1");
	paths.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	paths.append("dir2");
	paths.append("dir3/dir3_2/file3_2_1.txt");
	paths.append("dir3");
	paths.append("dir3/dir3_1");
	paths.append("dir3/dir3_2/dir3_2_1");
	paths.append("dir3/dir3_2");
	paths.append("dir1/dir1_2");
	paths.append("dir3/dir3_3");
	paths.append("dir1/dir1_1/dir1_1_1");
	paths.append("dir3/dir3_2/dir3_2_2");

	QDateTime date(QDateTime(QDate(2019, 3, 31)));

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, paths, date);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		if (path.indexOf(".") > 0) {
			folderView.addEntry(path, path.size(), date, path);
		}
		else {
			folderView.addDirEntry(path);
		}
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}

TEST(tst_TeArchiveFolderView, addFileEntry_skipAdd)
{
	//directry add deep entry
	QStringList paths;
	paths.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	paths.append("dir3/dir3_2/file3_2_1.txt");
	paths.append("dir1/file1_2.txt");
	paths.append("dir1/file1_1.txt");

	QDateTime date(QDateTime(QDate(2019, 3, 31)));

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, paths, date);

	TeArchiveFolderView folderView;
	for (auto& path : paths) {
		if (path.indexOf(".") > 0) {
			folderView.addEntry(path, path.size(), date, path);
		}
		else {
			folderView.addDirEntry(path);
		}
	}

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(folderView.list()->model());
	QStandardItem* item = model->item(0);
	checkEntries(&entries, item);
}
