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

#include <QDialog>

class QButtonGroup;
class QLineEdit;

class TeAskCreationModeDialog : public QDialog
{
	Q_OBJECT

public:
	enum CreateMode{
		MODE_CREATE_FOLDER = 0x01,
		MODE_RENAME_FILE   = 0x02,
	};
	Q_ENUM(CreateMode)
public:
	TeAskCreationModeDialog(QWidget *parent=Q_NULLPTR);
	virtual  ~TeAskCreationModeDialog();

	void setModeEnabled(CreateMode mode, bool flag);

	void setCreateMode(CreateMode mode);
	CreateMode createMode();

	void setTargetPath(const QString& path);
	QString path();

private:
	QButtonGroup* mp_createMode;
	QLineEdit*    mp_path;
};
