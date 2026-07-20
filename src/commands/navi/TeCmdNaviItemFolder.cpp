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

#include "TeCmdNaviItemFolder.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

#include <QFileInfo>

/**
 * @file TeCmdNaviItemFolder.cpp
 * @brief Declaration of TeCmdNaviItemFolder.
 * @ingroup commands
 */

const char* TeCmdNaviItemFolder::PARAM_OPEN_TARGET_DIR = "open_target_dir";

TeCmdNaviItemFolder::TeCmdNaviItemFolder()
{
}

TeCmdNaviItemFolder::~TeCmdNaviItemFolder()
{
}

bool TeCmdNaviItemFolder::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviItemFolder::type()
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


bool TeCmdNaviItemFolder::execute(TeViewStore* p_store)
{
	QString currentItem = getCurrentItem(p_store);
	if (currentItem.isEmpty()) {
		return true;
	}

	QFileInfo fileInfo(currentItem);
	if (!fileInfo.exists()) {
		return true;
	}

	if (cmdParam() && cmdParam()->contains(PARAM_OPEN_TARGET_DIR)) {
		QString parentFolder = fileInfo.absoluteFilePath();
		QString currentFolder = getCurrentFolder(p_store);
		if (parentFolder == currentFolder) {
			return true;
		}

		// Open the parent folder
		p_store->createFolderView(parentFolder, TeViewStore::TAB_LEFT);
	}else{
		QString parentFolder = fileInfo.absolutePath();
		QString currentFolder = getCurrentFolder(p_store);
		if (parentFolder == currentFolder) {
			return true;
		}

		// Open the parent folder
		p_store->createFolderView(parentFolder, TeViewStore::TAB_LEFT);
	}
	return true;
}
