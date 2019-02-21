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
