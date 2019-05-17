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
#include "TeTypes.h"

class QTreeWidgetItem;
class QSettings;
class TeTreeWidget;

class TeMenuSetting : public QDialog
{
	Q_OBJECT

public:
	TeMenuSetting(QWidget *parent);
	virtual ~TeMenuSetting();

	static void storeDefaultSettings();

public slots:
	void accept();

protected:
	static void storeDefaultMenuSettings(QSettings& settings);
	static void storeDefaultTreeMenuSettings(QSettings& settings);
	static void storeDefaultListMenuSettings(QSettings& settings);
	void loadSettings( QList<TeTreeWidget*>* p_itemList );
	void updateSettings();
	int storeMenuItemSettings(QSettings& settings, const QTreeWidgetItem* item, int indent, int index);

	void contextMenu(TeTreeWidget* tree, QTreeWidgetItem* item);
	void editEntryName(QTreeWidgetItem* item);
	TeTreeWidget* createNewMenu(const QString& name);
	QTreeWidgetItem* TeMenuSetting::createEntryItem(const QString& name, TeTypes::CmdId cmdId);
	QTreeWidgetItem* createFolderItem();
	QTreeWidgetItem* createSeparatorItem();
	QList<QTreeWidgetItem*> createCmdTreeItem();

private:
	enum {
		MENU_DISPLAY,
		MENU_DESCRIPTION,
		MENU_COMMAND_ID,
		MENU_REGISTER,
	};
	QList<TeTreeWidget*> m_itemList;
};
