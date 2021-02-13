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
			QString mpath = dlg.path();
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.createMode() == TeAskCreationModeDialog::MODE_RENAME_FILE) {
					//copy to modified path.
					bSuccess = copyFile(list.at(0), mpath);
				}
				else {
					//create new directory and copy to it.
					bSuccess = dir.mkpath(mpath);
					if (bSuccess) {
						bSuccess = copyFiles(list, mpath);
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
