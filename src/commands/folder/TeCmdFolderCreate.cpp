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
#include "TeCmdFolderCreate.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>

TeCmdFolderCreate::TeCmdFolderCreate()
{
}


TeCmdFolderCreate::~TeCmdFolderCreate()
{
}

bool TeCmdFolderCreate::execute(TeViewStore * p_store)
{
	// treeView時はtreeViewのカレントインデックス以下にフォルダを作成
	// treeViewにフォーカスがない場合は、ListViewをターゲットとしてフォルダを作成
	// ターゲットが存在しない場合はなにもやらない。

	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		//処理対象ビュー確定
		QString path;

		if (p_folder->tree()->hasFocus()) {
			QAbstractItemView* p_itemView = p_folder->tree();
			if (p_itemView->currentIndex().isValid()) {
				QFileSystemModel* model = qobject_cast<QFileSystemModel*>(p_itemView->model());
				if (model != Q_NULLPTR) {
					path = model->filePath(p_itemView->currentIndex());
				}
			}
		}
		else {
			path = p_folder->currentPath();
		}

		if (!path.isNull()) {
			//カレントターゲットを対象とする。
			QInputDialog dlg(p_store->mainWindow());
			dlg.setLabelText(QInputDialog::tr("Enter Folder name."));
			if (dlg.exec() == QInputDialog::Accepted) {
				QDir dir;
				if (!dir.mkdir(path + "/" + dlg.textValue())) {
					QMessageBox::warning(p_store->mainWindow(), TeFileFolderView::tr("Fail"), TeFileFolderView::tr("Failed: Create folder."));
				}

			}
		}
	}

	return true;
}
