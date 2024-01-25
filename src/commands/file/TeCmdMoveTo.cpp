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
#include "TeUtils.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "platform/platform_util.h"

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>



TeCmdMoveTo::TeCmdMoveTo()
{
}


TeCmdMoveTo::~TeCmdMoveTo()
{
}

bool TeCmdMoveTo::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdMoveTo::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_TOGGLE
		// TeTypes::CMD_TRIGGER_SELECTION

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_NONE

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_ARCHIVE
	);
}

QList<TeMenuParam> TeCmdMoveTo::menuParam()
{
	return QList<TeMenuParam>();
}

/**
* Move selected entry to folder.
*/
bool TeCmdMoveTo::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

	if (p_folder != nullptr) {
		QStringList paths;

		if (getSelectedItemList(p_store, &paths)) {
			//select folder.
			TeFilePathDialog dlg(p_store->mainWindow());
			dlg.setCurrentPath(p_folder->currentPath());
			dlg.setWindowTitle(TeFilePathDialog::tr("Move to"));
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

	if (dir.exists(path)) {
		bSuccess = moveFiles(list, path);
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
				bSuccess = moveFiles(list, path);
			}
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Move to following path failed.") + QString("\n") + path);
		msg.exec();
	}
}