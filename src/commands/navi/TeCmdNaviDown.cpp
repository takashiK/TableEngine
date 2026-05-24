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

#include "TeCmdNaviDown.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include <QKeyEvent>
#include <QCoreApplication>

/**
 * @file TeCmdNaviDown.cpp
 * @brief Declaration of TeCmdNaviDown.
 * @ingroup commands
 */


TeCmdNaviDown::TeCmdNaviDown()
{
}

TeCmdNaviDown::~TeCmdNaviDown()
{
}

bool TeCmdNaviDown::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviDown::type()
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


bool TeCmdNaviDown::execute(TeViewStore* p_store)
{
	switch(srcType()) {
	case TeTypes::WT_TREEVIEW:
		{
			//move down cusor in the tree view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto tree = folderview->tree();
				// send down key event to tree view
				QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
				QCoreApplication::sendEvent(tree, &downEvent);
			}
			break;
		}
	case TeTypes::WT_LISTVIEW:
		{
			//move down cusor in the list view
			auto folderview = p_store->currentFolderView();
			if (folderview) {
				auto list = folderview->list();
				// send down key event to list view
				QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
				QCoreApplication::sendEvent(list, &downEvent);
			}
			break;
		}
	default:
		break;
	}
	return true;
}
