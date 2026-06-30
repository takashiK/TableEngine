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

#include "TeCmdRename.h"
#include "TeViewStore.h"
#include "utils/TeUtils.h"
#include "widgets/TeArchiveFolderView.h"
#include "TeSettings.h"

#include <QStringList>
#include <QInputDialog>
#include <QFileInfo>
#include <QMessageBox>

/**
 * @file TeCmdRename.cpp
 * @brief Implementation of TeCmdRename.
 * @ingroup commands
 */

TeCmdRename::TeCmdRename()
{
}

TeCmdRename::~TeCmdRename()
{
}

bool TeCmdRename::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdRename::type()
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

bool TeCmdRename::execute(TeViewStore* p_store)
{
	QStringList paths;
	if (getSelectedItemList(p_store, &paths)) {
		TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
		if (p_arc != nullptr) {
			// Renaming inside an archive is only supported in writable mode; it
			// updates the staged entry keys rather than touching the filesystem.
			if (p_arc->isReadOnly()) {
				return true;
			}
			for (const auto& path : paths) {
				const int slash = path.lastIndexOf('/');
				const QString baseName = (slash >= 0) ? path.mid(slash + 1) : path;
				bool ok;
				QInputDialog inputDlg(p_store->mainWindow());
				inputDlg.setWindowTitle(QObject::tr("Rename"));
				inputDlg.setLabelText(QObject::tr("New name"));
				inputDlg.setTextValue(baseName);
				inputDlg.setMinimumWidth(TeSettings::dialogMinimumWidth());
				ok = (inputDlg.exec() == QDialog::Accepted);
				const QString newName = inputDlg.textValue();
				if (newName.isEmpty() || !ok) {
					return true;
				}
				if (baseName != newName) {
					p_arc->renameEntry(path, newName);
				}
			}
			return true;
		}

		for (const auto& path : paths) {
			QFileInfo info(path);
			bool ok;
			QInputDialog inputDlg(p_store->mainWindow());
			inputDlg.setWindowTitle(QObject::tr("Rename"));
			inputDlg.setLabelText(QObject::tr("New name"));
			inputDlg.setTextValue(info.fileName());
			inputDlg.setMinimumWidth(TeSettings::dialogMinimumWidth());
			ok = (inputDlg.exec() == QDialog::Accepted);
			const QString newName = inputDlg.textValue();
			if (newName.isEmpty() || !ok) {
				return true;
			}
			if (info.fileName() != newName) {
				bool result = QFile::rename(path, info.absolutePath() + "/" + newName);
				if (!result) {
					QMessageBox msg(p_store->mainWindow());
					msg.setWindowTitle(QObject::tr("Rename"));
					msg.setText(QObject::tr("Failed to rename file."));
					msg.setInformativeText(path);
					msg.setIcon(QMessageBox::Warning);
					msg.exec();
				}
			}
		}
	}
	return true;
}
