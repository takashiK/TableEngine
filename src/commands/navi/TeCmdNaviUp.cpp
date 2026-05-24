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

#include "TeCmdNaviUp.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"

#include <QKeyEvent>

/**
 * @file TeCmdNaviUp.cpp
 * @brief Declaration of TeCmdNaviUp.
 * @ingroup commands
 */


TeCmdNaviUp::TeCmdNaviUp()
{
}

TeCmdNaviUp::~TeCmdNaviUp()
{
}

bool TeCmdNaviUp::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviUp::type()
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


bool TeCmdNaviUp::execute(TeViewStore* p_store)
{
	switch(srcType()) {
	case TeTypes::WT_TREEVIEW:
		{
			//move up cusor in the tree view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto tree = folderview->tree();
				// send up key event to tree view
				QKeyEvent upEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
				QCoreApplication::sendEvent(tree, &upEvent);
			}
			break;
		}
	case TeTypes::WT_LISTVIEW:
		{
			//move up cusor in the list view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto list = folderview->list();
				// send up key event to list view
				QKeyEvent upEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
				QCoreApplication::sendEvent(list, &upEvent);
			}
			break;
		}
	default:
		break;
	}
	return true;
}
