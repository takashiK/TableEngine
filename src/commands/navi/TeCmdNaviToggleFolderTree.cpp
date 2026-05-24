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

#include "TeCmdNaviToggleFolderTree.h"
#include "utils/TeUtils.h"

#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"


/**
 * @file TeCmdNaviToggleFolderTree.cpp
 * @brief Declaration of TeCmdNaviToggleFolderTree.
 * @ingroup commands
 */


TeCmdNaviToggleFolderTree::TeCmdNaviToggleFolderTree()
{
}

TeCmdNaviToggleFolderTree::~TeCmdNaviToggleFolderTree()
{
}

bool TeCmdNaviToggleFolderTree::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviToggleFolderTree::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}


bool TeCmdNaviToggleFolderTree::execute(TeViewStore* p_store)
{
	TeFolderView* p_folderView = p_store->currentFolderView();
	if (p_folderView == nullptr) {
		return true;
	}

	TeFileTreeView* p_naviTree = p_store->navTreeView();
	if (p_naviTree != nullptr && !p_naviTree->isHidden()) {
		if( p_naviTree->hasFocus() ){
			p_folderView->list()->viewport()->setFocus();
		}
		else {
			p_naviTree->viewport()->setFocus();
		}
	}
	else{
		p_folderView->list()->viewport()->setFocus();
	}

	
	return true;
}
