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

#include "TeTypes.h"
#include "TeSettings.h"
#include "TeToolbarSetting.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"
#include "widgets/TeTreeWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QCursor>

/**
 * @file TeToolbarSetting.cpp
 * @brief Implementation of TeToolbarSetting.
 * @ingroup dialogs
 */

TeToolbarSetting::TeToolbarSetting(QWidget *parent)
	: QDialog(parent)
	, mp_toolbar(nullptr)
{
	setWindowTitle(tr("Toolbar Setting"));
	setMinimumWidth(TeSettings::dialogMinimumWidth());

	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* hbox   = new QHBoxLayout();

	// Left pane: toolbar preview (drag-and-drop target + internal move)
	mp_toolbar = new TeTreeWidget();
	mp_toolbar->setWindowTitle(tr("Toolbar"));
	mp_toolbar->setColumnCount(2);
	mp_toolbar->setIndentation(10);
	mp_toolbar->setAlternatingRowColors(true);
	mp_toolbar->headerItem()->setText(0, tr("Command"));
	mp_toolbar->headerItem()->setText(1, tr("Description"));
	mp_toolbar->setDragDropMode(QTreeWidget::DragDrop);
	mp_toolbar->setDefaultDropAction(Qt::MoveAction);
	mp_toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
	mp_toolbar->setRootIsDecorated(false);

	loadSettings();

	connect(mp_toolbar, &TeTreeWidget::customContextMenuRequested,
		[this](const QPoint& pos)
	{
		contextMenu(mp_toolbar, mp_toolbar->itemAt(pos));
	});

	// Right pane: command list (drag source only)
	QTreeWidget* cmd = new QTreeWidget();
	cmd->setColumnCount(2);
	cmd->setIndentation(10);
	cmd->setAlternatingRowColors(true);
	cmd->headerItem()->setText(0, tr("Command"));
	cmd->headerItem()->setText(1, tr("Description"));
	cmd->setDragDropMode(QTreeWidget::DragOnly);

	// Connect toolbar as buddy so drops from cmd go to mp_toolbar
	mp_toolbar->setBuddy(cmd);

	cmd->addTopLevelItems(createCmdTreeItem());

	hbox->addWidget(mp_toolbar);
	hbox->addWidget(cmd);
	layout->addLayout(hbox);

	// OK / Cancel / Reset buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeToolbarSetting::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(buttonBox, &QDialogButtonBox::clicked, [this, buttonBox](QAbstractButton* button) {
		if (buttonBox->standardButton(button) == QDialogButtonBox::Reset) {
			QSettings settings;
			settings.remove(SETTING_TOOLBAR);
			storeDefaultSettings(true);
			QDialog::accept();
		}
	});
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeToolbarSetting::~TeToolbarSetting()
{
}

void TeToolbarSetting::storeDefaultSettings(bool force)
{
	QSettings settings;
	if (settings.childGroups().contains(SETTING_TOOLBAR)) {
		if (!force) {
			return;
		}
		settings.remove(SETTING_TOOLBAR);
	}

	TeCommandFactory* factory = TeCommandFactory::factory();
	settings.beginGroup(SETTING_TOOLBAR);

	int index = 0;

	auto toolbarEntry = [&](TeTypes::CmdId cmdId) {
		const TeCommandInfoBase* p_info = factory->commandInfo(cmdId);
		if (p_info) {
			settings.setValue(
				QString("item%1").arg(index++, 3, 10, u'0'),
				QString("0,%1,%2").arg(p_info->name()).arg(static_cast<int>(p_info->cmdId())));
		}
	};
	auto separatorEntry = [&]() {
		settings.setValue(
			QString("item%1").arg(index++, 3, 10, u'0'),
			QString("0,---,%1").arg(static_cast<int>(TeTypes::CMDID_SPECIAL_SEPARATOR)));
	};

	toolbarEntry(TeTypes::CMDID_SYSTEM_FILE_NEW);
	separatorEntry();
	toolbarEntry(TeTypes::CMDID_SYSTEM_EDIT_CUT);
	toolbarEntry(TeTypes::CMDID_SYSTEM_EDIT_COPY);
	toolbarEntry(TeTypes::CMDID_SYSTEM_EDIT_PASTE);
	toolbarEntry(TeTypes::CMDID_SYSTEM_FILE_RENAME);
	separatorEntry();
	toolbarEntry(TeTypes::CMDID_SYSTEM_FILE_PROPERTY);
	toolbarEntry(TeTypes::CMDID_SYSTEM_FILE_RUN_WITH_COMMAND);
	separatorEntry();
	toolbarEntry(TeTypes::CMDID_SYSTEM_VIEW_FILTER);
	toolbarEntry(TeTypes::CMDID_SYSTEM_FOLDER_FIND);
	separatorEntry();
	toolbarEntry(TeTypes::CMDID_SYSTEM_TOOL_VIEW_FILE);
	toolbarEntry(TeTypes::CMDID_SYSTEM_TOOL_VIEW_BINARY);

	settings.endGroup();
}

void TeToolbarSetting::loadSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_TOOLBAR);

	for (const auto& key : settings.childKeys()) {
		QStringList values = settings.value(key).toString().split(',');
		if (values.size() < 3) continue;
		TeTypes::CmdId cmdId = static_cast<TeTypes::CmdId>(values[2].toInt());
		QString name = values[1];
		mp_toolbar->addTopLevelItem(createEntryItem(name, cmdId));
	}

	settings.endGroup();
}

void TeToolbarSetting::updateSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_TOOLBAR);
	settings.remove("");

	for (int i = 0; i < mp_toolbar->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = mp_toolbar->topLevelItem(i);
		settings.setValue(
			QString("item%1").arg(i, 3, 10, u'0'),
			QString("0,%1,%2")
				.arg(item->data(COL_REGISTER, Qt::DisplayRole).toString())
				.arg(item->data(COL_CMD_ID, Qt::DisplayRole).toInt()));
	}

	settings.endGroup();
}

QTreeWidgetItem* TeToolbarSetting::createEntryItem(const QString& name, TeTypes::CmdId cmdId)
{
	QString normalized = name;
	normalized.replace("&", "");
	QTreeWidgetItem* p_item = new QTreeWidgetItem();

	if (cmdId == TeTypes::CMDID_SPECIAL_SEPARATOR) {
		p_item->setData(COL_DISPLAY,  Qt::DisplayRole, "---");
		p_item->setData(COL_CMD_ID,   Qt::DisplayRole, static_cast<int>(TeTypes::CMDID_SPECIAL_SEPARATOR));
		p_item->setData(COL_REGISTER, Qt::DisplayRole, "---");
	}
	else {
		const TeCommandInfoBase* p_info = TeCommandFactory::factory()->commandInfo(cmdId);
		if (p_info == nullptr) {
			p_item->setData(COL_DISPLAY,  Qt::DisplayRole,    tr("Invalid Command Id"));
			p_item->setData(COL_DISPLAY,  Qt::DecorationRole, QIcon(":/TableEngine/warning.png"));
			p_item->setData(COL_CMD_ID,   Qt::DisplayRole,    static_cast<int>(TeTypes::CMDID_NONE));
			p_item->setData(COL_REGISTER, Qt::DisplayRole,    tr("Invalid Command Id"));
		}
		else {
			p_item->setData(COL_DISPLAY,  Qt::DisplayRole,    normalized);
			p_item->setData(COL_DISPLAY,  Qt::DecorationRole, p_info->icon());
			p_item->setData(COL_DESCRIPTION, Qt::DisplayRole, p_info->description());
			p_item->setData(COL_CMD_ID,   Qt::DisplayRole,    static_cast<int>(p_info->cmdId()));
			p_item->setData(COL_REGISTER, Qt::DisplayRole,    name);
		}
	}
	return p_item;
}

QTreeWidgetItem* TeToolbarSetting::createSeparatorItem()
{
	return createEntryItem("---", TeTypes::CMDID_SPECIAL_SEPARATOR);
}

QList<QTreeWidgetItem*> TeToolbarSetting::createCmdTreeItem()
{
	QList<QTreeWidgetItem*> treeItem;
	QList<std::pair<QString, TeTypes::CmdId>>  list = TeCommandFactory::groupList();

	for (const auto& item : list) {
		auto cmdName = QString(item.first).replace("&", "");
		QTreeWidgetItem* rootItem = new QTreeWidgetItem({ cmdName });
		rootItem->addChildren(createCmdItem(item.second));
		treeItem.append(rootItem);
	}
	return treeItem;
}

QList<QTreeWidgetItem *> TeToolbarSetting::createCmdItem(TeTypes::CmdId cmdGroup)
{
	QList<QTreeWidgetItem*> items;
	TeCommandFactory* factory = TeCommandFactory::factory();

	QList<const TeCommandInfoBase*>  list = factory->commandGroup(cmdGroup);
	for (const auto& info : list) {
		QString name = info->name();
		name.replace("&","");
		
		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setData(COL_DISPLAY, Qt::DecorationRole, info->icon());
		item->setData(COL_DISPLAY, Qt::DisplayRole, name);
		item->setData(COL_DESCRIPTION, Qt::DisplayRole, info->description());
		item->setData(COL_CMD_ID, Qt::DisplayRole, info->cmdId());
		item->setData(COL_REGISTER, Qt::DisplayRole, info->name());
		items.append(item);
	}

	return items;
}

void TeToolbarSetting::contextMenu(TeTreeWidget* tree, QTreeWidgetItem* item)
{
	QMenu menu(tree);
	QAction* a_sep = menu.addAction(tr("Insert Separator"));
	if (item != nullptr) {
		menu.addSeparator();
		QAction* a_del = menu.addAction(tr("Delete"));
		connect(a_sep, &QAction::triggered, [this, item, tree](bool /*checked*/) {
			int idx = tree->indexOfTopLevelItem(item);
			if (idx >= 0)
				tree->insertTopLevelItem(idx, createSeparatorItem());
			else
				tree->addTopLevelItem(createSeparatorItem());
		});
		connect(a_del, &QAction::triggered, [item](bool /*checked*/) {
			delete item;
		});
	}
	else {
		connect(a_sep, &QAction::triggered, [this, tree](bool /*checked*/) {
			tree->addTopLevelItem(createSeparatorItem());
		});
	}
	menu.exec(QCursor::pos());
}

void TeToolbarSetting::accept()
{
	updateSettings();
	QDialog::accept();
}
