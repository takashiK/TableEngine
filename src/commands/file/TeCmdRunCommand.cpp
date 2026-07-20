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

#include "TeCmdRunCommand.h"
#include "TeViewStore.h"
#include "utils/TeUtils.h"
#include "utils/TeToolCommand.h"
#include "dialogs/TeCommandInputDialog.h"

/**
 * @file TeCmdRunCommand.cpp
 * @brief Implementation of TeCmdRunCommand.
 * @ingroup commands
 */

const char* TeCmdRunCommand::PARAM_COMMAND = "command";
const char* TeCmdRunCommand::PARAM_WITH_SHELL = "shell";
const char* TeCmdRunCommand::PARAM_OUTPUT = "output";

TeCmdRunCommand::TeCmdRunCommand()
{
}

TeCmdRunCommand::~TeCmdRunCommand()
{
}

bool TeCmdRunCommand::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdRunCommand::type()
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

bool TeCmdRunCommand::execute(TeViewStore* p_store)
{
	//Macro
	// %F : Replace current file path.
	// %f : Replace current file name.
	// %M : Replace selected file paths.
	// %m : Replace selected file names.
	// %P : Replace current folder path.

	QString command;
	bool shell = false;
	bool output = false;
	if (cmdParam()->contains(PARAM_COMMAND)) {
		command = cmdParam()->value(PARAM_COMMAND).toString();
		shell = cmdParam()->value(PARAM_WITH_SHELL).toBool();
		output = cmdParam()->value(PARAM_OUTPUT).toBool();
	}
	else {
		TeCommandInputDialog dlg;
		dlg.setWindowTitle(QObject::tr("Run command"));
		if (dlg.exec() == QDialog::Accepted) {
			command = dlg.command();
			shell = dlg.shell();
			output = dlg.output();
		}
	}

	TeToolCommand::runCommand(p_store, command, shell, output);

	return true;
}

