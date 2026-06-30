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

#include "TeCmdExtract.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>

#include "TeViewStore.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TePasswordDialog.h"
#include "utils/TeArchive.h"
#include "utils/TeUtils.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"

/**
 * @file TeCmdExtract.cpp
 * @brief Implementation of TeCmdExtract.
 * @ingroup commands
 */

TeCmdExtract::TeCmdExtract() {
}

TeCmdExtract::~TeCmdExtract() {
}

bool TeCmdExtract::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam) {
    NOT_USED(p_store);
    NOT_USED(p_cmdParam);
    return false;
}

QFlags<TeTypes::CmdType> TeCmdExtract::type() {
    return QFlags<TeTypes::CmdType>(
        TeTypes::CMD_TRIGGER_NORMAL
        // TeTypes::CMD_TRIGGER_SELECT

        | TeTypes::CMD_CATEGORY_TREE | TeTypes::CMD_CATEGORY_LIST | TeTypes::CMD_CATEGORY_OTHER

        | TeTypes::CMD_TARGET_FILE | TeTypes::CMD_TARGET_DIRECTORY);
}

/**
 * Copy selected files to target directory
 */
bool TeCmdExtract::execute(TeViewStore* p_store) {
    QStringList paths;

    if (getSelectedItemList(p_store, &paths)) {

		TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());
        if (p_folder != nullptr) {
            // get distination folder.
            QStringList extraFlags = {QObject::tr("Create new folder by archive file name.")};

            TeFilePathDialog dlg(p_store->mainWindow(), extraFlags);
            dlg.setExtraFlag(0, true);

            dlg.setCurrentPath(p_folder->currentPath());
            dlg.setFavorites(getFavorites());
            dlg.setHistory(p_folder->getPathHistory());

            dlg.setWindowTitle(TeFilePathDialog::tr("Extract to"));
            if (dlg.exec() == QDialog::Accepted) {
                if (dlg.targetPath().isEmpty()) {
                    QMessageBox msg(p_store->mainWindow());
                    msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
                    msg.setText(QObject::tr("Faild ExtractTo Function.\nTarget path is not set."));
                    msg.exec();
                } else {
                    extractArchives(p_store, paths, dlg.targetPath(), dlg.getExtraFlag(0));
                }
            }
        }
		else{
			TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
			if (p_arc != nullptr) {
				// get distination folder.
				TeFilePathDialog dlg(p_store->mainWindow());

				QFileInfo info(p_arc->archivePath());
				dlg.setCurrentPath(info.absolutePath());
				dlg.setFavorites(getFavorites());
				dlg.setTargetPath(info.absolutePath());

				dlg.setWindowTitle(TeFilePathDialog::tr("Extract to"));
				if (dlg.exec() == QDialog::Accepted) {
					if (dlg.targetPath().isEmpty()) {
						QMessageBox msg(p_store->mainWindow());
						msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
						msg.setText(QObject::tr("Faild ExtractTo Function.\nTarget path is not set."));
						msg.exec();
					} else {
						QString targetPath = dlg.targetPath();
						QString basePath = p_arc->currentPath();
						extractArchiveSelectionToPath(p_store, basePath, paths, targetPath);
					}
				}
			}
		}
    }

    return true;
}
