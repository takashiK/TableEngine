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
#include "TeTypes.h"

/**
 * @file TeToolbarSetting.h
 * @brief Declaration of TeToolbarSetting.
 * @ingroup dialogs
 */

class QTreeWidgetItem;
class TeTreeWidget;

class TeToolbarSetting : public QDialog
{
	Q_OBJECT

public:
	TeToolbarSetting(QWidget *parent);
	virtual ~TeToolbarSetting();

	static void storeDefaultSettings(bool force = false);

public slots:
	void accept();

protected:
	void loadSettings();
	void updateSettings();

	void contextMenu(TeTreeWidget* tree, QTreeWidgetItem* item);
	QTreeWidgetItem* createEntryItem(const QString& name, TeTypes::CmdId cmdId);
	QTreeWidgetItem* createSeparatorItem();
	QList<QTreeWidgetItem*> createCmdTreeItem();
	QList<QTreeWidgetItem*> createCmdItem( TeTypes::CmdId cmdGroup );


private:
	enum {
		COL_DISPLAY = 0,
		COL_DESCRIPTION = 1,
		COL_CMD_ID  = 2,
		COL_REGISTER = 3,
	};

	TeTreeWidget* mp_toolbar;
};
