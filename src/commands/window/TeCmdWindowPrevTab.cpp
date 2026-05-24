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

#include "TeCmdWindowPrevTab.h"
#include "utils/TeUtils.h"

#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"

/**
 * @file TeCmdWindowPrevTab.cpp
 * @brief Declaration of TeCmdWindowPrevTab.
 * @ingroup commands
 */


TeCmdWindowPrevTab::TeCmdWindowPrevTab()
{
}

TeCmdWindowPrevTab::~TeCmdWindowPrevTab()
{
}

bool TeCmdWindowPrevTab::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdWindowPrevTab::type()
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


bool TeCmdWindowPrevTab::execute(TeViewStore* p_store)
{
	if (p_store == nullptr) return true;

	auto pos = p_store->currentTabPlace();
	auto index = p_store->currentTabIndex();
	auto folders = p_store->getFolderViews(pos);
	if (folders.isEmpty()) return true;

	if (index <= 0) {
		index = folders.size() - 1;
	}
	else if (index > folders.size() - 1) {
		index = folders.size() - 1;
	}
	else {
		index--;	
	}
	p_store->setCurrentFolderView(folders.at(index));

	return true;
}
