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
* 選択中ファイルを指定フォルダにコピーする。
*/
bool TeCmdCopyTo::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		//コピー対象確定
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
			for (const QModelIndex& index : indexList)
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
			//コピー先フォルダ確定
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
