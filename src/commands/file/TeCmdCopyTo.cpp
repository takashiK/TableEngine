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
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "TeCmdCopyTo.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "platform/platform_util.h"

#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

TeCmdCopyTo::TeCmdCopyTo()
{
}


TeCmdCopyTo::~TeCmdCopyTo()
{
}

/**
* Copy selected files to target directory
*/
bool TeCmdCopyTo::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		//get selected files
		QAbstractItemView* p_itemView = nullptr;
		if (p_folder->tree()->hasFocus()) {
			p_itemView = p_folder->tree();
		}
		else {
			p_itemView = p_folder->list();
		}

		QFileSystemModel* model = qobject_cast<QFileSystemModel*>(p_itemView->model());
		QStringList paths;

		if (p_itemView->selectionModel()->hasSelection()) {
			//target selected files.
			QModelIndexList indexList = p_itemView->selectionModel()->selectedIndexes();
			for (const QModelIndex& index : indexList)
			{
				if (index.column() == 0) {
					paths.append(model->filePath(index));
				}
			}
		}
		else {
			//no files selected. so use current file.
			if (p_itemView->currentIndex().isValid()) {
				paths.append(model->filePath(p_itemView->currentIndex()));
			}
		}

		if (!paths.isEmpty()) {
			//get distination folder.
			TeFilePathDialog dlg(p_store->mainWindow());
			dlg.setCurrentPath(p_folder->currentPath());
			dlg.setWindowTitle(TeFilePathDialog::tr("Copy to"));
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.targetPath().isEmpty()) {
					QMessageBox msg(p_store->mainWindow());
					msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
					msg.setText(QObject::tr("Faild CopyTo Function.\nTarget path is not set."));
					msg.exec();
				}
				else {
					copyItems(p_store,paths, dlg.targetPath());
				}
			}
		}
	}

	return true;
}

void TeCmdCopyTo::copyItems(TeViewStore* p_store, const QStringList & list, const QString & path)
{
	//指定ディレクトリが存在しない場合作成するか問い合わせる。
	QDir dir;

	bool bSuccess = true;

	if (dir.exists(path)) {
		bSuccess = copyFiles(list, path);
	}
	else {
		QFileInfo info(path);
		if (list.count() == 1 && info.dir().exists()) {
			//Rename or CreateFolder
			TeAskCreationModeDialog dlg(p_store->mainWindow());
			dlg.setTargetPath(path);
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.createMode() == TeAskCreationModeDialog::MODE_RENAME_FILE) {
					//リネームコピー実行
					bSuccess = copyFile(list.at(0), path);
				}
				else {
					//新規フォルダ作成してからコピー
					bSuccess = dir.mkpath(path);
					if (bSuccess) {
						bSuccess = copyFiles(list, path);
					}
				}
			}
		}
		else {
			TeAskCreationModeDialog dlg(p_store->mainWindow());
			dlg.setTargetPath(path);
			dlg.setModeEnabled(TeAskCreationModeDialog::MODE_RENAME_FILE, false);
			if (dlg.exec() == QDialog::Accepted) {
				//Create Folder
				bSuccess = dir.mkpath(path);
				if (bSuccess) {
					bSuccess = copyFiles(list, path);
				}
			}
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Copy to following path failed.") + QString("\n") + path);
		msg.exec();
	}
}
