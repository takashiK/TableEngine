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

/*!
	Create folder. rule of create point is below.
	Target to TreeView : Create folder under current index in treeview.
	Target to not treeview : Create folder in listview.
 */
bool TeCmdFolderCreate::execute(TeViewStore * p_store)
{

	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
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

		if (!path.isEmpty()) {
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
