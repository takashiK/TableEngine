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

#include "TeCmdToolFileEdit.h"
#include "TeViewStore.h"
#include "TeSettings.h"
#include "utils/TeUtils.h"
#include "utils/TeToolCommand.h"

#include <QSettings>
#include <QMessageBox>

/**
 * @file TeCmdToolFileEdit.cpp
 * @brief Declaration of TeCmdToolFileEdit.
 * @ingroup commands
 */


TeCmdToolFileEdit::TeCmdToolFileEdit()
{
}

TeCmdToolFileEdit::~TeCmdToolFileEdit()
{
}

bool TeCmdToolFileEdit::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdToolFileEdit::type()
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


bool TeCmdToolFileEdit::execute(TeViewStore* p_store)
{
	QStringList selected;
	if (!getSelectedFileList(p_store, &selected) || selected.isEmpty()) {
		return true;
	}
	const QString targetFile = selected.first();

	// 1./2. Check tools/user/tool01..MAX_USER_TOOLS for a suffix match.
	QString command = TeToolCommand::findUserToolCommand(targetFile);

	// 3. Fall back to the tool configured for the detected file type.
	if (command.isEmpty()) {
		QSettings settings;
		switch (getFileType(targetFile)) {
		case TE_FILE_TEXT:
			command = settings.value(SETTING_TOOLS_TEXT_EDIT).toString();
			break;
		case TE_FILE_IMAGE:
			command = settings.value(SETTING_TOOLS_IMAGE_EDIT).toString();
			break;
		default:
			if(isTextFile(targetFile)) {
				command = settings.value(SETTING_TOOLS_TEXT_EDIT).toString();
			}
			break;
		}
	}

	// 4. No matching tool: confirm falling back to the binary editor.
	if (command.isEmpty()) {
		const int result = QMessageBox::warning(p_store->mainWindow(),
			QObject::tr("No editor configured"),
			QObject::tr("No editor tool is configured for this file.\nOpen with the binary editor instead?"),
			QMessageBox::Ok, QMessageBox::Cancel);
		if (result != QMessageBox::Ok) {
			return true;
		}
		QSettings settings;
		command = settings.value(SETTING_TOOLS_BINARY_EDIT).toString();
	}

	TeToolCommand::runCommand(p_store, command, false);

	return true;
}
