/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "TeCmdSelectToggle.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"



TeCmdSelectToggle::TeCmdSelectToggle()
{
}


TeCmdSelectToggle::~TeCmdSelectToggle()
{
}

/**
* カレントのTeFileListView上での選択状態を反転する。
*/
bool TeCmdSelectToggle::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileListView* p_list = p_folder->list();
		QAbstractItemModel* model = p_list->model();
		QModelIndex root = p_list->rootIndex();

		QModelIndex startIndex = model->index(0, 0, root);
		QModelIndex endIndex = model->index(model->rowCount(root) -1, model->columnCount(root)-1, root);

		QItemSelection selection(startIndex, endIndex);
		p_list->selectionModel()->select(selection, QItemSelectionModel::Toggle);
	}

	return true;
}
