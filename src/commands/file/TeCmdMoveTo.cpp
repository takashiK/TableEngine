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
#include "TeCmdMoveTo.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "platform/platform_util.h"

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>



TeCmdMoveTo::TeCmdMoveTo()
{
}


TeCmdMoveTo::~TeCmdMoveTo()
{
}

/**
* 選択中ファイルを指定フォルダに移動する。
*/
bool TeCmdMoveTo::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		//移動対象確定
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
			//選択済コンテンツを対象とする。
			QModelIndexList indexList = p_itemView->selectionModel()->selectedIndexes();
			for(const QModelIndex& index : indexList)
			{
				if (index.column() == 0) {
					paths.append(model->filePath(index));
				}
			}
		}
		else {
			//未選択時はカレントターゲットを対象とする。
			if (p_itemView->currentIndex().isValid()) {
				paths.append(model->filePath(p_itemView->currentIndex()));
			}
		}

		if (!paths.isEmpty()) {
			//移動先フォルダ確定
			TeFilePathDialog dlg(p_store->mainWindow());
			dlg.setCurrentPath(p_folder->currentPath());
			dlg.setWindowTitle(TeFilePathDialog::tr("Move to"));
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.targetPath().isEmpty()) {
					QMessageBox msg(p_store->mainWindow());
					msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
					msg.setText(QObject::tr("Faild MoveTo Function.\nTarget path is not set."));
					msg.exec();
				}
				else {
					moveItems(p_store, paths, dlg.targetPath());
				}
			}
		}
	}

	return true;
}

void TeCmdMoveTo::moveItems(TeViewStore* p_store, const QStringList & list, const QString & path)
{
	//指定ディレクトリが存在しない場合作成するか問い合わせる。
	QDir dir;

	bool bSuccess = true;

	if (dir.exists(path)) {
		bSuccess = moveFiles(list, path);
	}
	else {
		QFileInfo info(path);
		TeAskCreationModeDialog dlg(p_store->mainWindow());
		dlg.setTargetPath(path);
		dlg.setModeEnabled(TeAskCreationModeDialog::MODE_RENAME_FILE, false);
		if (dlg.exec() == QDialog::Accepted) {
			//Create Folder
			bSuccess = dir.mkpath(path);
			if (bSuccess) {
				bSuccess = moveFiles(list, path);
			}
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Move to following path failed.") + QString("\n") + path);
		msg.exec();
	}
}