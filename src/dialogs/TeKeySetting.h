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

#include "TeTypes.h"

#include <QDialog>
#include <QList>

class QTreeWidget;
class QTreeWidgetItem;

class TeKeySetting : public QDialog
{
	Q_OBJECT

public:
	TeKeySetting(QWidget *parent);
	virtual ~TeKeySetting();

	void updateSettings();
	static void storeDefaultSettings();

public slots:
	void accept();

protected:
	QList<QTreeWidgetItem*> createKeyTreeItem();
	QList<QTreeWidgetItem*> createKeyItem( const QList<int>& list);
	QList<QTreeWidgetItem*> createCmdTreeItem();
	QList<QTreeWidgetItem*> createCmdItem( TeTypes::CmdId cmdGroup );

private:
	QTreeWidget* mp_list;
};
