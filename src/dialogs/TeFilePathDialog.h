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
class QComboBox;

class TeFilePathDialog : public QDialog
{
	Q_OBJECT

public:
	TeFilePathDialog(QWidget *parent = nullptr);
	virtual  ~TeFilePathDialog();

	void setCurrentPath(const QString& path);
	QString currentPath();

	void setTargetPath(const QString& path);
	QString targetPath();

//	void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	QComboBox* mp_combo;
	QString    m_currentPath;
};
