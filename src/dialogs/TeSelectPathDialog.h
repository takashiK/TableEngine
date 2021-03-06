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

class QLineEdit;
class TeFileTreeView;

class TeSelectPathDialog : public QDialog
{
	Q_OBJECT

public:
	TeSelectPathDialog(QWidget *parent);
	virtual  ~TeSelectPathDialog();

	QString targetPath();

public slots:
	void setTargetPath(const QString& path);

private:
	QLineEdit*  mp_edit;
	TeFileTreeView* mp_tree;
};
