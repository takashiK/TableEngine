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

#include "TeCmdNaviLeft.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include <QKeyEvent>
#include <QCoreApplication>

/**
 * @file TeCmdNaviLeft.cpp
 * @brief Declaration of TeCmdNaviLeft.
 * @ingroup commands
 */


TeCmdNaviLeft::TeCmdNaviLeft()
{
}

TeCmdNaviLeft::~TeCmdNaviLeft()
{
}

bool TeCmdNaviLeft::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviLeft::type()
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


bool TeCmdNaviLeft::execute(TeViewStore* p_store)
{
	switch(srcType()) {
	case TeTypes::WT_TREEVIEW:
		{
			//move left cusor in the tree view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto tree = folderview->tree();
				// send left key event to tree view
				QKeyEvent leftEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
				QCoreApplication::sendEvent(tree, &leftEvent);
			}
			break;
		}
	case TeTypes::WT_LISTVIEW:
		{
			//move left cusor in the list view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto list = folderview->list();
				// send left key event to list view
				QKeyEvent leftEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
				QCoreApplication::sendEvent(list, &leftEvent);
			}
			break;
		}
	default:
		break;
	}
	return true;
}
