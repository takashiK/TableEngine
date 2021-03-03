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

#include "TeCmdFolderOpenUnder.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"

#include <QFileSystemModel>
#include <QModelIndex>

TeCmdFolderOpenUnder::TeCmdFolderOpenUnder()
{
}


TeCmdFolderOpenUnder::~TeCmdFolderOpenUnder()
{
}

bool TeCmdFolderOpenUnder::isAvailable()
{
	return true;
}

bool TeCmdFolderOpenUnder::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->expand(p_tree->currentIndex());
			if(p_tree->model()->hasChildren(p_tree->currentIndex()))
				openUnder(p_tree, p_tree->currentIndex());
		}
	}

	return true;
}

void TeCmdFolderOpenUnder::openUnder(TeFileTreeView* p_tree, const QModelIndex& index)
{
	QAbstractItemModel* model = p_tree->model();
	if (model->hasChildren(index)) {
		p_tree->expand(index);
		for (int i = 0; i < model->rowCount(index); i++) {
			QModelIndex child = model->index(i, 0, index);
			if(model->hasChildren(child))
				openUnder(p_tree, child);
		}
	}
}
