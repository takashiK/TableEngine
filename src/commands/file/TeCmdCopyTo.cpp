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

#include "TeCmdCopyTo.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include "TeViewStore.h"
#include "dialogs/TeAskCreationModeDialog.h"
#include "dialogs/TeFilePathDialog.h"
#include "platform/TeFileOperationManager.h"
#include "utils/TeUtils.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"
#include "TeSettings.h"


/**
 * @file TeCmdCopyTo.cpp
 * @brief Implementation of TeCmdCopyTo.
 * @ingroup commands
 */

TeCmdCopyTo::TeCmdCopyTo() {
}

TeCmdCopyTo::~TeCmdCopyTo() {
}

bool TeCmdCopyTo::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam) {
    NOT_USED(p_store);
    NOT_USED(p_cmdParam);
    return false;
}

QFlags<TeTypes::CmdType> TeCmdCopyTo::type() {
    return QFlags<TeTypes::CmdType>(
        TeTypes::CMD_TRIGGER_NORMAL
        // TeTypes::CMD_TRIGGER_SELECT

        | TeTypes::CMD_CATEGORY_TREE | TeTypes::CMD_CATEGORY_LIST | TeTypes::CMD_CATEGORY_OTHER

        | TeTypes::CMD_TARGET_FILE | TeTypes::CMD_TARGET_DIRECTORY);
}

/**
 * Copy selected files to target directory
 */
bool TeCmdCopyTo::execute(TeViewStore* p_store) {
    QStringList paths;

    QString targetPath;
    if (QSettings().value(SETTING_GENERAL_CopyToOppositePane, true).toBool()) {
        int tabPlace = p_store->currentTabPlace() == TeViewStore::TAB_RIGHT ? TeViewStore::TAB_LEFT : TeViewStore::TAB_RIGHT;
        TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->getFolderView(tabPlace));
        if (p_folder != nullptr) {
            targetPath = p_folder->currentPath();
        }
    }

    if (getSelectedItemList(p_store, &paths)) {
        TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

        if (p_folder != nullptr) {
            // get distination folder.
            TeFilePathDialog dlg(p_store->mainWindow());
            dlg.setCurrentPath(p_folder->currentPath());
            dlg.setFavorites(getFavorites());
            dlg.setHistory(p_folder->getPathHistory());
            if(!targetPath.isEmpty()) {
                dlg.setTargetPath(targetPath);
            }

            dlg.setWindowTitle(TeFilePathDialog::tr("Copy to"));
            if (dlg.exec() == QDialog::Accepted) {
                if (dlg.targetPath().isEmpty()) {
                    QMessageBox msg(p_store->mainWindow());
                    msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
                    msg.setText(QObject::tr("Faild CopyTo Function.\nTarget path is not set."));
                    msg.exec();
                } else {
                    copyItems(p_store, paths, dlg.targetPath());
                }
            }
        }
		else {
			TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
			if (p_arc != nullptr) {
				// get distination folder.
				TeFilePathDialog dlg(p_store->mainWindow());

				QFileInfo info(p_arc->archivePath());
				dlg.setCurrentPath(info.absolutePath());
				dlg.setFavorites(getFavorites());
                if(!targetPath.isEmpty()) {
                    dlg.setTargetPath(targetPath);
                }else{
                    dlg.setTargetPath(info.absolutePath());
                }

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

void TeCmdCopyTo::copyItems(TeViewStore* p_store, const QStringList& list, const QString& path) {
    QDir dir;

    bool bSuccess = true;
    const QString errorText = QObject::tr("Copy to following path failed.") + QString("\n") + path;
    TeFileOperationManager* mgr = p_store->fileOperationManager();

    if (dir.exists(path)) {
        mgr->copyFiles(list, path, errorText);
    } else {
        QFileInfo info(path);
        if (list.size() == 1 && info.dir().exists()) {
            // Rename or CreateFolder
            TeAskCreationModeDialog dlg(p_store->mainWindow());
            dlg.setTargetPath(path);
            QString mpath = dlg.path();
            if (dlg.exec() == QDialog::Accepted) {
                if (dlg.createMode() == TeAskCreationModeDialog::MODE_RENAME_FILE) {
                    // copy to modified path.
                    mgr->copyFile(list.at(0), mpath, errorText);
                } else {
                    // create new directory and copy to it.
                    bSuccess = dir.mkpath(mpath);
                    if (bSuccess) {
                        mgr->copyFiles(list, mpath, errorText);
                    }
                }
            }
        } else {
            TeAskCreationModeDialog dlg(p_store->mainWindow());
            dlg.setTargetPath(path);
            dlg.setModeEnabled(TeAskCreationModeDialog::MODE_RENAME_FILE, false);
            if (dlg.exec() == QDialog::Accepted) {
                // Create Folder
                bSuccess = dir.mkpath(path);
                if (bSuccess) {
                    mgr->copyFiles(list, path, errorText);
                }
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
