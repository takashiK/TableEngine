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

bool TeCmdFolderOpenUnder::isActive( TeViewStore* p_store )
{
	return false;
}

QFlags<TeTypes::CmdType> TeCmdFolderOpenUnder::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_TOGGLE
		// TeTypes::CMD_TRIGGER_SELECTION

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_NONE

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_ARCHIVE
	);
}

QList<TeMenuParam> TeCmdFolderOpenUnder::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdFolderOpenUnder::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
//			p_tree->expand(p_tree->currentIndex());
//			if(p_tree->model()->hasChildren(p_tree->currentIndex()))
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
