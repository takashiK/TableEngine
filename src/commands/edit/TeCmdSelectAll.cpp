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

#include "TeCmdSelectAll.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"

#include <QAbstractItemModel>

TeCmdSelectAll::TeCmdSelectAll()
{
}


TeCmdSelectAll::~TeCmdSelectAll()
{
}

bool TeCmdSelectAll::isAvailable()
{
	return true;
}

/**
	Toggle select all or not selected.
	If entry is already selected then entry change to "not select".
 */
bool TeCmdSelectAll::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileListView* p_list = p_folder->list();
		QAbstractItemModel* model = p_list->model();
		QModelIndex root = p_list->rootIndex();

		QModelIndex startIndex = model->index(0, 0, root);
		QModelIndex endIndex = model->index(model->rowCount(root) - 1, model->columnCount(root) - 1, root);

		QItemSelection selection(startIndex, endIndex);
		if (p_list->selectionModel()->hasSelection()) {
			p_list->selectionModel()->select(selection, QItemSelectionModel::Clear);
		}
		else {
			p_list->selectionModel()->select(selection, QItemSelectionModel::Select);
		}

		if (".." == model->data(startIndex).toString()) {
			p_list->selectionModel()->select(startIndex, QItemSelectionModel::Deselect);
		}
	}

	return true;
}