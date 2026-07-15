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

#include "TeCmdAddQuickAccess.h"
#include "utils/TeUtils.h"
#include "dialogs/TeFilePathDialog.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeDriveBar.h"
#include "TeViewStore.h"

#include <QFileInfo>

/**
 * @file TeCmdAddQuickAccess.cpp
 * @brief Declaration of TeCmdAddQuickAccess.
 * @ingroup commands
 */


TeCmdAddQuickAccess::TeCmdAddQuickAccess()
{
}

TeCmdAddQuickAccess::~TeCmdAddQuickAccess()
{
}

bool TeCmdAddQuickAccess::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdAddQuickAccess::type()
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


bool TeCmdAddQuickAccess::execute(TeViewStore* p_store)
{
	TeFilePathDialog dlg;
	dlg.setCurrentPath(getCurrentFolder(p_store));
	dlg.setTargetPath(getCurrentFolder(p_store));
	dlg.setFavorites(getFavorites());
	dlg.setHistory(p_store->currentFolderView()->getPathHistory());
	if (dlg.exec() == QDialog::Accepted) {
		QFileInfo info(dlg.targetPath());
		if (info.exists() && info.isDir()) {
			p_store->driveBar()->addQuickAccess(info.absoluteFilePath());
			p_store->driveBar()->storeQuickAccesses();
		}
	}
	return true;
}
