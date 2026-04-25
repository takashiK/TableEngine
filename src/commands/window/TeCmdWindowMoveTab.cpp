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

#include "TeCmdWindowMoveTab.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

TeCmdWindowMoveTab::TeCmdWindowMoveTab()
{
}

TeCmdWindowMoveTab::~TeCmdWindowMoveTab()
{
}

bool TeCmdWindowMoveTab::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdWindowMoveTab::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdWindowMoveTab::type()
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


bool TeCmdWindowMoveTab::execute(TeViewStore* p_store)
{
	if (!p_store)
		return true;
	
	auto folderView = p_store->currentFolderView();
	if (!folderView)
		return true;

	p_store->moveFolderView(folderView, p_store->currentTabPlace() == TeViewStore::TAB_LEFT ? TeViewStore::TAB_RIGHT : TeViewStore::TAB_LEFT);

	return true;
}
