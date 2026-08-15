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
#include "widgets/TeFileFolderView.h"

#include <QFileInfo>
#include <QDir>

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

	QString targetFolder = fileInfo.absolutePath();
	if (cmdParam() && cmdParam()->contains(PARAM_OPEN_TARGET_DIR)) {
		// If the command parameter specifies a target folder, use it instead of the item's parent folder.
		// ie. value of PARAM_OPEN_TARGET_DIR is "true" only. 
		targetFolder = fileInfo.absoluteFilePath();
	}
	
	QString currentFolder = getCurrentFolder(p_store);
	if (targetFolder == currentFolder) {
		return true;
	}

	// get root path of the target folder
	QDir dir(targetFolder);
	while (dir.cdUp()) {
		if (dir.isRoot()) {
			break;
		}
	}
	QString rootPath = dir.absolutePath();

	// Open the root path of the target folder in the left tab panel.
	// and set the current folder to the target folder.
	auto p_view = p_store->createFolderView(rootPath, TeViewStore::TAB_LEFT);
	if (p_view) {
		p_view->setCurrentPath(targetFolder);
	}

	return true;
}
