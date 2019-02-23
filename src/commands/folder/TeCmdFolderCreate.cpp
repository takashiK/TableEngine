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

		if (p_itemView->currentIndex().isValid()) {
			//カレントターゲットを対象とする。
			QInputDialog dlg(p_store->mainWindow());
			dlg.setLabelText(QInputDialog::tr("Enter Folder name."));
			if (dlg.exec() == QInputDialog::Accepted) {
				QString name = dlg.textValue();
				QDir dir;
				if (!dir.mkdir(model->filePath(p_itemView->currentIndex()) + "/" + dlg.textValue())) {
					QMessageBox::warning(p_store->mainWindow(), TeFileFolderView::tr("Fail"), TeFileFolderView::tr("Failed: Create folder."));
				}

			}
		}
		else if(!p_folder->tree()->hasFocus()){
			//ツリーじゃない場合はカレントフォルダをターゲットにする
			QInputDialog dlg(p_store->mainWindow());
			dlg.setLabelText(QInputDialog::tr("Enter Folder name."));
			if (dlg.exec() == QInputDialog::Accepted) {
				QString name = dlg.textValue();
				QDir dir;
				if (!dir.mkdir(p_folder->currentPath() + "/" + dlg.textValue())) {
					QMessageBox::warning(p_store->mainWindow(), TeFileFolderView::tr("Fail"), TeFileFolderView::tr("Failed: Create folder."));
				}
			}
		}
	}

	return true;
}
