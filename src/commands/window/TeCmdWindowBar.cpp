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

#include "TeCmdWindowBar.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

TeCmdWindowBar::TeCmdWindowBar()
{
}

TeCmdWindowBar::~TeCmdWindowBar()
{
}

bool TeCmdWindowBar::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdWindowBar::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if (p_store && p_cmdParam) {
		int bar = p_cmdParam->value("SELECTION").toInt();

		switch (bar) {
		case ToolBar:
			return p_store->isToolBarVisible();
		case DriveBar:
			return p_store->isDriveBarVisible();
		case StatusBar:
			return p_store->isStatusBarVisible();
		case Navigation:
			return p_store->isNavigationVisible();
		case Detail:
			return p_store->isDetailVisible();
		default:
			break;
		}

	}
	return false;
}

QFlags<TeTypes::CmdType> TeCmdWindowBar::type()
{
	return QFlags<TeTypes::CmdType>(
		// TeTypes::CMD_TRIGGER_NORMAL
		TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}


bool TeCmdWindowBar::execute(TeViewStore* p_store)
{
	if (!p_store) {
		return true;
	}

	int bar = cmdParam()->value("SELECTION").toInt();
	switch (bar) {
	case ToolBar:
		p_store->setToolBarVisible(!p_store->isToolBarVisible());
		break;
	case DriveBar:
		p_store->setDriveBarVisible(!p_store->isDriveBarVisible());
		break;
	case StatusBar:
		p_store->setStatusBarVisible(!p_store->isStatusBarVisible());
		break;
	case Navigation:
		p_store->setNavigationVisible(!p_store->isNavigationVisible());
		break;
	case Detail:
		p_store->setDetailVisible(!p_store->isDetailVisible());
		break;
	default:
		break;
	}
	return true;
}
