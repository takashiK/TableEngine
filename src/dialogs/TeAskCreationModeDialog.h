/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
