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

#include "TeCmdViewLayout.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

/**
 * @file TeCmdViewLayout.cpp
 * @brief Implementation of TeCmdViewLayout.
 * @ingroup commands
 */

TeCmdViewLayout::TeCmdViewLayout()
{
}

TeCmdViewLayout::~TeCmdViewLayout()
{
}

bool TeCmdViewLayout::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdViewLayout::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if(p_store && p_cmdParam) {
		LayoutType layout = LayoutType(p_cmdParam->value("SELECTION").toInt());
		switch(layout) {
			case SMALL_ICON:
				return p_store->viewMode() == TeTypes::FILEVIEW_SMALL_ICON;
			case LARGE_ICON:
				return p_store->viewMode() == TeTypes::FILEVIEW_LARGE_ICON;
			case HUGE_ICON:
				return p_store->viewMode() == TeTypes::FILEVIEW_HUGE_ICON;
			case DETAIL_LIST:
				return p_store->viewMode() == TeTypes::FILEVIEW_DETAIL_LIST;
		}
	}
	return false;
}

QFlags<TeTypes::CmdType> TeCmdViewLayout::type()
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


bool TeCmdViewLayout::execute(TeViewStore* p_store)
{
	if(p_store && cmdParam()) {
		LayoutType layout = LayoutType(cmdParam()->value("SELECTION").toInt());
		switch(layout) {
			case SMALL_ICON:
				p_store->setViewMode(TeTypes::FILEVIEW_SMALL_ICON);
				break;
			case LARGE_ICON:
				p_store->setViewMode(TeTypes::FILEVIEW_LARGE_ICON);
				break;
			case HUGE_ICON:
				p_store->setViewMode(TeTypes::FILEVIEW_HUGE_ICON);
				break;
			case DETAIL_LIST:
				p_store->setViewMode(TeTypes::FILEVIEW_DETAIL_LIST);
				break;
		}
	}
	return true;
}
