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

#include "TeCmdMoveTo.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "utils/TeUtils.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "platform/TeFileOperationManager.h"
#include "TeSettings.h"

#include <QMainWindow>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QSettings>

/**
 * @file TeCmdMoveTo.cpp
 * @brief Implementation of TeCmdMoveTo.
 * @ingroup commands
 */

TeCmdMoveTo::TeCmdMoveTo()
{
}


TeCmdMoveTo::~TeCmdMoveTo()
{
}

bool TeCmdMoveTo::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdMoveTo::type()
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

/**
* Move selected entry to folder.
*/
bool TeCmdMoveTo::execute(TeViewStore* p_store)
{
    QString targetPath;
    if (QSettings().value(SETTING_GENERAL_CopyToOppositePane, true).toBool()) {
        int tabPlace = p_store->currentTabPlace() == TeViewStore::TAB_RIGHT ? TeViewStore::TAB_LEFT : TeViewStore::TAB_RIGHT;
        TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->getFolderView(tabPlace));
        if (p_folder != nullptr) {
            targetPath = p_folder->currentPath();
        }
    }

	TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

	if (p_folder != nullptr) {
		QStringList paths;

		if (getSelectedItemList(p_store, &paths)) {
			//select folder.
			TeFilePathDialog dlg(p_store->mainWindow());
			dlg.setCurrentPath(p_folder->currentPath());
			dlg.setFavorites(getFavorites());
			dlg.setHistory(p_folder->getPathHistory());
			dlg.setWindowTitle(TeFilePathDialog::tr("Move to"));
            if(!targetPath.isEmpty()) {
                dlg.setTargetPath(targetPath);
            }
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.targetPath().isEmpty()) {
					QMessageBox msg(p_store->mainWindow());
					msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
					msg.setText(QObject::tr("Faild MoveTo Function.\nTarget path is not set."));
					msg.exec();
				}
				else {
					moveItems(p_store, paths, dlg.targetPath());
				}
			}
		}
	}

	return true;
}

void TeCmdMoveTo::moveItems(TeViewStore* p_store, const QStringList & list, const QString & path)
{
	QDir dir;

	bool bSuccess = true;
	const QString errorText = QObject::tr("Move to following path failed.") + QString("\n") + path;
	TeFileOperationManager* mgr = p_store->fileOperationManager();

	if (dir.exists(path)) {
		mgr->moveFiles(list, path, errorText);
	}
	else {
		QFileInfo info(path);
		TeAskCreationModeDialog dlg(p_store->mainWindow());
		dlg.setTargetPath(path);
		dlg.setModeEnabled(TeAskCreationModeDialog::MODE_RENAME_FILE, false);
		if (dlg.exec() == QDialog::Accepted) {
			//Create Folder
			bSuccess = dir.mkpath(path);
			if (bSuccess) {
				mgr->moveFiles(list, path, errorText);
			}
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(errorText);
		msg.exec();
	}
}
