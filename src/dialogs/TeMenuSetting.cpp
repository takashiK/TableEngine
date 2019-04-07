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
#include "TeTypes.h"
#include "TeSettings.h"
#include "TeMenuSetting.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"
#include "widgets/TeTreeWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QCombobox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QInputDialog>
#include <QSettings>

TeMenuSetting::TeMenuSetting(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout*  hbox = new QHBoxLayout();

	QVBoxLayout* vbox = new QVBoxLayout();

	loadSettings(&m_itemList);

	QComboBox* combo = new QComboBox();
	QTreeWidget* cmd = new QTreeWidget();
	vbox->addWidget(combo);
	for (int i = 0; i < m_itemList.count(); i++) {
		combo->addItem(m_itemList[i]->windowTitle(), i);
		vbox->addWidget(m_itemList[i]);
		m_itemList[i]->setBuddy(cmd);
		m_itemList[i]->setHidden(i!=0);
	}
	connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
		for (int i = 0; i < m_itemList.size(); i++) {
			if (i == index) {
				m_itemList[i]->setHidden(false);
			}
			else {
				m_itemList[i]->setHidden(true);
			}
		}
	});

	hbox->addLayout(vbox);

	cmd->setWindowTitle("Command List");
	cmd->setColumnCount(2);
	cmd->setIndentation(10);
	cmd->setAlternatingRowColors(true);
	cmd->headerItem()->setText(0, tr("Command"));
	cmd->headerItem()->setText(1, tr("Description"));
	cmd->setDragDropMode(QTreeWidget::DragOnly);

	cmd->addTopLevelItems(createCmdTreeItem());

	hbox->addWidget(cmd);
	layout->addLayout(hbox);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(buttonBox, &QDialogButtonBox::clicked, [this,buttonBox,combo](QAbstractButton *button) {
		if (buttonBox->standardButton(button) == QDialogButtonBox::Reset) {
			QSettings settings;
			settings.beginGroup(SETTING_MENU);
			settings.beginGroup(QString("menuGroup0%1").arg(combo->currentIndex()));
			settings.remove("");
			settings.endGroup();
			settings.remove(QString("menuGroup0%1").arg(combo->currentIndex()));
			settings.endGroup();
			storeDefaultSettings();
			QDialog::accept();
		}
	});
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeMenuSetting::~TeMenuSetting()
{

}

void TeMenuSetting::storeDefaultSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_MENU);
	if (!settings.contains("menuGroup00"))
		storeDefaultMenuSettings(settings);
	if (!settings.contains("menuGroup01"))
		storeDefaultTreeMenuSettings(settings);
	if (!settings.contains("menuGroup02"))
		storeDefaultListMenuSettings(settings);
	settings.endGroup();
}

void defaultEntry(QSettings& settings, const QString& key,  const QString& value)
{
	settings.setValue(key, settings.value(key, value));
}

void menuEntry(QSettings& settings, int index, int indent, TeTypes::CmdId cmdId)
{
	TeCommandInfoBase*p_info = TeCommandFactory::factory()->commandInfo(cmdId);
	defaultEntry(settings, QString("menu%1").arg(index, 3, 10, QChar('0')), QString("%1,%2,%3").arg(indent).arg(p_info->name()).arg(p_info->cmdId()));
}

void specialEntry(QSettings& settings, int index, int indent, TeTypes::CmdId cmdId, const QString& str)
{
	defaultEntry(settings, QString("menu%1").arg(index, 3, 10, QChar('0')), QString("%1,%2,%3").arg(indent).arg(str).arg(cmdId));
}

void TeMenuSetting::storeDefaultMenuSettings(QSettings& settings)
{
	QList<QPair<QString, TeTypes::CmdId>>  list = TeCommandFactory::groupList();
	TeCommandFactory* factory = TeCommandFactory::factory();
	int index = 0;
	int indent = 0;

	QString group = QString("menuGroup00");
	defaultEntry(settings, group, tr("Menu Bar"));
	settings.beginGroup(group);
	for (const auto& groupItem : list) {
		specialEntry(settings, index++, indent, TeTypes::CMDID_SPECIAL_FOLDER, groupItem.first);
		for (const auto& item : factory->commandGroup(groupItem.second)) {
			menuEntry(settings, index++, indent + 1, item->cmdId());
		}
	}
	settings.endGroup();
}

void TeMenuSetting::storeDefaultTreeMenuSettings(QSettings& settings )
{
	TeCommandFactory* factory = TeCommandFactory::factory();
	int index = 0;
	int indent = 0;
	QString group = QString("menuGroup01");
	defaultEntry(settings, group, tr("Popup menu on TreeView"));

	settings.beginGroup(group);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_OPEN_UNDER);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ALL);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_UNDER);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_ALL);
	specialEntry(settings, index++, indent, TeTypes::CMDID_SPECIAL_SEPARATOR,"---");
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_CREATE_FOLDER);
	settings.endGroup();
}

void TeMenuSetting::storeDefaultListMenuSettings(QSettings& settings )
{
	TeCommandFactory* factory = TeCommandFactory::factory();
	int index = 0;
	int indent = 0;
	QString group = QString("menuGroup02");
	defaultEntry(settings, group, tr("Popup menu on ListView"));

	settings.beginGroup(group);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_OPEN_UNDER);
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_UNDER);
	specialEntry(settings, index++, indent, TeTypes::CMDID_SPECIAL_SEPARATOR, "---");
	menuEntry(settings, index++, indent, TeTypes::CMDID_SYSTEM_FOLDER_CREATE_FOLDER);
	settings.endGroup();
}

QTreeWidgetItem* TeMenuSetting::createEntryItem(const QString& name, TeTypes::CmdId cmdId)
{
	QString normalized = name;
	normalized.replace("&", "");
	QTreeWidgetItem* p_item = new QTreeWidgetItem();
	if (cmdId == TeTypes::CMDID_SPECIAL_SEPARATOR) {
		p_item->setData(MENU_DISPLAY, Qt::DisplayRole, "---");
		p_item->setData(MENU_COMMAND_ID, Qt::DisplayRole, TeTypes::CMDID_SPECIAL_SEPARATOR);
		p_item->setData(MENU_REGISTER, Qt::DisplayRole, "---");
	}
	else if (cmdId == TeTypes::CMDID_SPECIAL_FOLDER) {
		p_item->setData(MENU_DISPLAY, Qt::DisplayRole, normalized);
		p_item->setData(MENU_COMMAND_ID, Qt::DisplayRole, TeTypes::CMDID_SPECIAL_FOLDER);
		p_item->setData(MENU_REGISTER, Qt::DisplayRole, name);
	}
	else {
		TeCommandInfoBase* p_info = TeCommandFactory::factory()->commandInfo(cmdId);
		if (p_info == nullptr) {
			p_item->setData(MENU_DISPLAY, Qt::DisplayRole, tr("Invalid Command Id"));
			p_item->setData(MENU_DISPLAY, Qt::DecorationRole, QIcon(":/TableEngine/warning.png"));
			p_item->setData(MENU_DESCRIPTION, Qt::DisplayRole, "");
			p_item->setData(MENU_COMMAND_ID, Qt::DisplayRole, TeTypes::CMDID_NONE);
			p_item->setData(MENU_REGISTER, Qt::DisplayRole, tr("Invalid Command Id"));
		}
		else {
			p_item->setData(MENU_DISPLAY, Qt::DisplayRole, normalized);
			p_item->setData(MENU_DISPLAY, Qt::DecorationRole, p_info->icon());
			p_item->setData(MENU_DESCRIPTION, Qt::DisplayRole, p_info->description());
			p_item->setData(MENU_COMMAND_ID, Qt::DisplayRole, p_info->cmdId());
			p_item->setData(MENU_REGISTER, Qt::DisplayRole, name);
		}
	}
	return p_item;
}

void TeMenuSetting::loadSettings(QList<TeTreeWidget*>* p_itemList)
{
	QSettings settings;
	TeCommandFactory* factory = TeCommandFactory::factory();

	settings.beginGroup(SETTING_MENU);
	for (const auto& group : settings.childGroups()) {
		TeTreeWidget* p_tree = createNewMenu(settings.value(group).toString());
		settings.beginGroup(group);
		int prvIndent = 0;
		QTreeWidgetItem* p_parentItem = nullptr;
		QTreeWidgetItem* p_prvItem = nullptr;
		for (const auto& key : settings.childKeys()) {
			QStringList values = settings.value(key).toString().split(',');
			int indent = values[0].toInt();
			TeTypes::CmdId cmdId = static_cast<TeTypes::CmdId>(values[2].toInt());
			QString name = values[1];

			if ( (p_prvItem != nullptr) && (indent > prvIndent) ) {
				//change child group
				p_parentItem = p_prvItem;
			}
			else if ( (p_parentItem != nullptr) && (indent < prvIndent) ) {
				//change parent group
				p_parentItem = p_parentItem->parent();
			}

			p_prvItem = nullptr;

			QTreeWidgetItem* p_item = createEntryItem(values[1],cmdId);
			if (cmdId == TeTypes::CMDID_SPECIAL_FOLDER) {
				p_prvItem = p_item;
			}

			prvIndent = indent;

			if (p_parentItem != nullptr) {
				p_parentItem->addChild(p_item);
			}
			else {
				p_tree->addTopLevelItem(p_item);
			}
		}
		settings.endGroup();
		p_itemList->append(p_tree);
	}
	settings.endGroup();
}

void TeMenuSetting::updateSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_MENU);

	settings.remove("");

	int index = 0;
	int indent = 0;
	QTreeWidgetItem* item;

	for (int i = 0; i < m_itemList.count(); i++ ) {
		QString group = QString("menuGroup%1").arg(i,2,10,QChar('0'));
		settings.setValue(group, m_itemList[i]->windowTitle());

		settings.beginGroup(group);
		index = 0;

		for (int j = 0; j < m_itemList[i]->topLevelItemCount(); j++) {
			item = m_itemList[i]->topLevelItem(j);
			settings.setValue(QString("menu%1").arg(index++, 3, 10, QChar('0')),
				QString("%1,%2,%3")
				.arg(indent)
				.arg(item->data(MENU_REGISTER, Qt::DisplayRole).toString())
				.arg(item->data(MENU_COMMAND_ID, Qt::DisplayRole).toInt()));
			if (item->childCount() > 0) {
				index = storeMenuItemSettings(settings, item, indent + 1, index);
			}
		}

		settings.endGroup();
	}

	settings.endGroup();
}
int TeMenuSetting::storeMenuItemSettings(QSettings& settings, const QTreeWidgetItem* itemRoot, int indent, int index)
{
	
	for (int i = 0; i < itemRoot->childCount(); i++) {
		QTreeWidgetItem* item = itemRoot->child(i);
		settings.setValue(QString("menu%1").arg(index++, 3, 10, QChar('0')), 
			QString("%1,%2,%3")
			.arg(indent)
			.arg(item->data(MENU_REGISTER,Qt::DisplayRole).toString())
			.arg(item->data(MENU_COMMAND_ID,Qt::DisplayRole).toInt()));
		if (item->childCount() > 0) {
			index = storeMenuItemSettings(settings, item, indent + 1, index);
		}
	}
	return index;
}


void TeMenuSetting::contextMenu(TeTreeWidget * tree, QTreeWidgetItem* item)
{
	QMenu menu(tree);
	QAction* a_add = menu.addAction(tr("Add Folder"));
	QAction* a_sep = menu.addAction(tr("Add Separator"));
	if (item != nullptr) {
		menu.addSeparator();
		QAction* a_edit = menu.addAction(tr("Edit"));
		QAction* a_del = menu.addAction(tr("Delete"));
		connect(a_add, &QAction::triggered, [this,item, tree](bool checked) {QTreeWidgetItem* folder = createFolderItem(); if(folder!=nullptr) item->addChild(folder); });
		connect(a_sep, &QAction::triggered, [this, item, tree](bool checked) {item->addChild(createSeparatorItem()); });
		connect(a_edit,&QAction::triggered, [this, item, tree](bool checked) {editEntryName(item); });
		connect(a_del, &QAction::triggered, [this, item, tree](bool checked) {tree->removeItemWidget(item, 0); delete item; });
	}
	else {
		connect(a_add, &QAction::triggered, [this, item, tree](bool checked) {QTreeWidgetItem* folder = createFolderItem(); if (folder != nullptr) tree->addTopLevelItem(folder); });
		connect(a_sep, &QAction::triggered, [this, item, tree](bool checked) {tree->addTopLevelItem(createSeparatorItem());  });
	}

	menu.exec(QCursor::pos());
}

void TeMenuSetting::editEntryName(QTreeWidgetItem * item)
{
	QInputDialog dlg(this);
	dlg.setLabelText(tr("Enter New name."));
	dlg.setTextValue(item->data(MENU_REGISTER, Qt::DisplayRole).toString());
	if (dlg.exec() == QInputDialog::Accepted) {
		QString name = dlg.textValue();
		item->setData(MENU_DISPLAY, Qt::DisplayRole, name.replace("&", ""));
		item->setData(MENU_REGISTER, Qt::DisplayRole, name);
	}
}

TeTreeWidget * TeMenuSetting::createNewMenu(const QString & name)
{
	TeTreeWidget* tree = new TeTreeWidget();

	tree->setWindowTitle(name);
	tree->setColumnCount(2);
	tree->setIndentation(10);
	tree->setAlternatingRowColors(true);
	tree->headerItem()->setText(0, tr("Command"));
	tree->headerItem()->setText(1, tr("Description"));
	tree->setDragDropMode(QTreeWidget::DragDrop);
	tree->setDefaultDropAction(Qt::MoveAction);
	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->setRootIndex(tree->model()->index(0, 0));

	connect(tree, &TeTreeWidget::customContextMenuRequested,
		[this, tree](const QPoint& pos)
	{
		contextMenu(tree, tree->itemAt(pos));
	});

	return tree;
}

QTreeWidgetItem * TeMenuSetting::createFolderItem()
{
	QInputDialog dlg(this);
	dlg.setLabelText(tr("Enter Folder name."));
	if (dlg.exec() == QInputDialog::Accepted) {
		QTreeWidgetItem* item = createEntryItem(dlg.textValue(), TeTypes::CMDID_SPECIAL_FOLDER);
		return item;
	}
	return nullptr;
}

QTreeWidgetItem * TeMenuSetting::createSeparatorItem()
{
	return createEntryItem("---",TeTypes::CMDID_SPECIAL_SEPARATOR);
}

QList<QTreeWidgetItem*> TeMenuSetting::createCmdTreeItem()
{
	QList<QTreeWidgetItem*> treeItem;
	QList<QPair<QString, TeTypes::CmdId>>  list = TeCommandFactory::groupList();

	for (const auto& item : list) {
		QString name = item.first;
		QTreeWidgetItem* rootItem = createEntryItem(name, TeTypes::CMDID_SPECIAL_FOLDER);

		QList<TeCommandInfoBase*>  cmdList = TeCommandFactory::factory()->commandGroup(item.second);
		for (const auto& info : cmdList) {
			QString name = info->name();
			QTreeWidgetItem* item = createEntryItem(info->name(), info->cmdId());
			if (item->flags().testFlag(Qt::ItemIsDropEnabled))
				item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
			rootItem->addChild(item);
		}

		treeItem.append(rootItem);
	}
	return treeItem;
}

void TeMenuSetting::accept()
{
	updateSettings();
	QDialog::accept();
}
