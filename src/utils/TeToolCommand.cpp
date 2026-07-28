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

#include "TeToolCommand.h"
#include "TeViewStore.h"
#include "TeSettings.h"
#include "utils/TeUtils.h"

#include <QProcess>
#include <QFileInfo>
#include <QStringList>
#include <QSettings>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QStringDecoder>
#include <qlightterminal.h>

/**
 * @file TeToolCommand.cpp
 * @brief Implementation of TeToolCommand.
 * @ingroup utility
 */

namespace {

	/** @brief Wraps @p value in double quotes when it contains a space. */
	QString quoteIfNeeded(const QString& value)
	{
		return value.contains(QLatin1Char(' ')) ? QString("\"%1\"").arg(value) : value;
	}

} // namespace

bool TeToolCommand::isValidFormat(const QString& commandTemplate)
{
	if (commandTemplate.trimmed().isEmpty()) {
		return true; // Not configured yet - treated as valid.
	}
	const QStringList tokens = QProcess::splitCommand(commandTemplate);
	return !tokens.isEmpty() && !tokens.first().isEmpty();
}

QString TeToolCommand::expandMacros(const QString& commandTemplate, TeViewStore* p_store, bool* p_hasTarget)
{
	QString result = commandTemplate;
	bool hasTarget = true;

	if (result.contains(QLatin1String("%F")) || result.contains(QLatin1String("%f"))) {
		QString current = getCurrentItem(p_store);
		if (current.isEmpty() || QFileInfo(current).isDir()) {
			current.clear();
			hasTarget = false; // %F/%f specified but no target file.
		}
		if (result.contains(QLatin1String("%F"))) {
			result.replace(QLatin1String("%F"), quoteIfNeeded(current));
		}
		if (result.contains(QLatin1String("%f"))) {
			result.replace(QLatin1String("%f"), quoteIfNeeded(QFileInfo(current).fileName()));
		}
	}

	if (result.contains(QLatin1String("%M")) || result.contains(QLatin1String("%m"))) {
		QStringList selected;
		getSelectedFileList(p_store, &selected);
		if (selected.isEmpty()) {
			hasTarget = false; // %M/%m specified but no target file.
		}

		if (result.contains(QLatin1String("%M"))) {
			QStringList quoted;
			for (const QString& path : selected) {
				quoted << quoteIfNeeded(path);
			}
			result.replace(QLatin1String("%M"), quoted.join(QLatin1Char(' ')));
		}
		if (result.contains(QLatin1String("%m"))) {
			QStringList names;
			for (const QString& path : selected) {
				names << quoteIfNeeded(QFileInfo(path).fileName());
			}
			result.replace(QLatin1String("%m"), names.join(QLatin1Char(' ')));
		}
	}

	if (result.contains(QLatin1String("%P"))) {
		result.replace(QLatin1String("%P"), quoteIfNeeded(getCurrentFolder(p_store)));
	}

	if (p_hasTarget != nullptr) {
		*p_hasTarget = hasTarget;
	}
	return result;
}

QString TeToolCommand::workingDirectory(TeViewStore* p_store)
{
	return getCurrentFolder(p_store);
}

void TeToolCommand::runCommand(TeViewStore* p_store, const QString& commandTemplate, bool shell, OUTPUT_MODE mode)
{
	if (commandTemplate.isEmpty()) {
		return;
	}

	bool hasTarget = true;
	const QString commandline = expandMacros(commandTemplate, p_store, &hasTarget);
	if (!hasTarget) {
		return; // %F/%f/%M/%m specified but no target file exists: do nothing.
	}

	QString command;
	QStringList environment = QProcess::systemEnvironment();
	QString workingDir = workingDirectory(p_store);


	// Resolve the configured shell only when requested. If "shell" is enabled
	// but no shell command is configured, fall back to direct execution
	// (same behaviour as shell == false).
	QSettings settings;
	const QString shellCommand = shell ? settings.value(SETTING_COMMAND_Shell).toString() : QString();

	QStringList args = QProcess::splitCommand(commandline);
	if (!args.isEmpty()) {
		if (!shellCommand.isEmpty()) {
			// execute with the configured shell
			const QString shellArg = settings.value(SETTING_COMMAND_ShellArg).toString();
			if (!shellArg.isEmpty()) {
				args.prepend(shellArg);
			}
			command = shellCommand;
		} else {
			// execute without shell
			command = args.takeFirst();
		}
	}

	if(mode == OUTPUT_TERMINAL){
		QLightTerminal* terminal = new QLightTerminal();
		terminal->setWindowTitle(QObject::tr("Run command"));
		terminal->setMinimumSize(600, 400);
		p_store->registerFloatingWidget(terminal);
		terminal->startProcess(command, args, workingDir, environment);
		terminal->show();
	}else{
		//run command
		QProcess process;
		process.setProgram(command);
		process.setArguments(args);
		process.setEnvironment(environment);
		process.setWorkingDirectory(workingDir);
		process.start(QProcess::ReadOnly);

		if (!process.waitForStarted()) {
			// TODO: shell variation
			QMessageBox::critical(p_store->mainWindow(), QObject::tr("Run command"), QObject::tr("Failed to start command."));
		}
		else {
			process.waitForFinished();
			if (mode == OUTPUT_STDOUT) {
				QPlainTextEdit* edit = new QPlainTextEdit();

				QByteArray out = process.readAllStandardOutput();
				QString codecName = detectTextCodec(out, QStringList({ "UTF-8","Shift_JIS","EUC-JP","ISO-2022-JP" }));
				QStringDecoder decoder(codecName.toLatin1().constData());
				if (!decoder.isValid()) {
					decoder = QStringDecoder("UTF-8");
				}
				QString outText = decoder(out);

				edit->setPlainText(outText);
				edit->setReadOnly(true);
				edit->setMinimumSize(600, 400);
				p_store->registerFloatingWidget(edit);
				edit->show();
			}
		}
	}
}

QString TeToolCommand::findUserToolCommand(const QString& path)
{
	const QString suffix = QFileInfo(path).suffix();

	QSettings settings;
	settings.beginGroup(SETTING_TOOLS_USER);
	QString result;
	for (int i = 1; i <= TeSettings::MAX_USER_TOOLS; ++i) {
		const QString key = QString("tool%1").arg(i, 2, 10, QLatin1Char('0'));
		if (!settings.contains(key)) {
			continue;
		}
		const QStringList fields = settings.value(key).toString().split(QLatin1Char(';'));
		if (fields.isEmpty()) {
			continue;
		}
		const QString command = fields.first().trimmed();
		for (int j = 1; j < fields.size(); ++j) {
			const QString entrySuffix = fields.at(j).trimmed();
			if (!entrySuffix.isEmpty() && entrySuffix.compare(suffix, Qt::CaseInsensitive) == 0) {
				result = command;
				break;
			}
		}
		if (!result.isEmpty()) {
			break;
		}
	}
	settings.endGroup();
	return result;
}
