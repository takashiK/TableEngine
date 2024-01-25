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
#include "TeUtils.h"
#include "dialogs/TeCommandInputDialog.h"

#include <QProcess>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QString>
#include <QTextCodec>

TeCmdRunCommand::TeCmdRunCommand()
{
}

TeCmdRunCommand::~TeCmdRunCommand()
{
}

bool TeCmdRunCommand::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdRunCommand::type()
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

QList<TeMenuParam> TeCmdRunCommand::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdRunCommand::execute(TeViewStore* p_store)
{
	//Macro
	// %F : Replace current file name.
	// %M : Replace selected file names.
	// %P : Replace current folder path.

	QString command;
	bool shell = false;
	bool output = false;
	if (cmdParam()->contains("command")) {
		command = cmdParam()->value("command").toString();
		shell = cmdParam()->value("shell").toBool();
		output = cmdParam()->value("output").toBool();
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

	if (!command.isEmpty()) {
		//replace %F to current file
		if (command.contains("%F")) {
			command.replace("%F", getCurrentItem(p_store));
		}

		//replace %M to selected file list
		if (command.contains("%M")) {
			QStringList selected;
			if (getSelectedItemList(p_store, &selected)) {
				command.replace("%M", selected.join(" "));
			}
		}

		//replace %P to current folder path
		if (command.contains("%P")) {
			command.replace("%P", getCurrentFolder(p_store));
		}
		
		//run command
		QProcess process;
		process.setWorkingDirectory(getCurrentFolder(p_store));

		if (shell) {
			command = "cmd.exe /c " + command;
		}

		process.startCommand(command, QProcess::ReadOnly);

		if (!process.waitForStarted()) {
			// TODO: shell variation
			QMessageBox::critical(p_store->mainWindow(), QObject::tr("Run command"), QObject::tr("Failed to start command."));
		}
		else {
			process.waitForFinished();
			if (output) {
				QPlainTextEdit* edit = new QPlainTextEdit();

				QByteArray out = process.readAllStandardOutput();
				QString codecName = detectTextCodec(out, QStringList({ "UTF-8","Shift_JIS","EUC-JP","ISO-2022-JP" }));
				QTextCodec* codec = QTextCodec::codecForName(codecName.toLatin1());
				if (!codec) {
					codecName = "UTF-8";
					codec = QTextCodec::codecForName(codecName.toLatin1());
				}

				QString outText = codec->toUnicode(out);

				edit->setPlainText(outText);
				edit->setReadOnly(true);
				p_store->registerFloatingWidget(edit);
				edit->show();
			}
		}
	}

	return true;
}
