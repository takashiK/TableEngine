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

#pragma once

#include <QPlainTextEdit>

/**
 * @class TeTerminalWidget
 * @brief QPlainTextEdit subclass for terminal-like text display.
 * @ingroup widgets
 *
 * @details Provides a terminal-like text display using QPlainTextEdit.
 */
class IPtyProcess;
class QStringDecoder;

class TeTerminalWidget : public QPlainTextEdit
{
	Q_OBJECT

public:
	TeTerminalWidget(QWidget *parent = nullptr);
	virtual ~TeTerminalWidget();

	bool startProcess(const QString &executable,
					  const QStringList &arguments,
					  const QString &workingDir,
					  QStringList environment);
	bool isRunning() const;

	void loadSettings();

	void setANSIByteArray(const QByteArray &byteArray);

public slots:
	void exitProcess();

signals:
	void aboutToClose();

protected:

private:
	IPtyProcess *mp_ptyProcess = nullptr;
	QStringDecoder *mp_decoder = nullptr;

	bool m_isRunning = false;
};
