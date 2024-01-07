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

#include "TeKeySetting.h"
#include "TeTypes.h"
#include "TeSettings.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QCheckBox>
#include <QKeySequence>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QSettings>
#include <QString>

static void changeKeyTree(int key, QCheckBox* ctrl, QCheckBox* shift, QTreeWidget* tree)
{
	switch (key) {
	case Qt::CTRL:
		if (ctrl->checkState() == Qt::Checked) { shift->setCheckState(Qt::Unchecked); }
		break;
	case Qt::SHIFT:
		if (shift->checkState() == Qt::Checked) { ctrl->setCheckState(Qt::Unchecked); }
		break;
	}

	if ((ctrl->checkState() == Qt::Unchecked) && (shift->checkState() == Qt::Unchecked)) {
		//Normal
		tree->setRootIndex(tree->model()->index(0, 0));
		tree->setCurrentIndex(tree->model()->index(0, 0, tree->model()->index(0, 0)));
	}
	else if ((ctrl->checkState() == Qt::Checked) && (shift->checkState() == Qt::Unchecked)) {
		//Ctrl
		tree->setRootIndex(tree->model()->index(1, 0));
		tree->setCurrentIndex(tree->model()->index(0, 0, tree->model()->index(1, 0)));
	}
	else if ((ctrl->checkState() == Qt::Unchecked) && (shift->checkState() == Qt::Checked)) {
		//Shift
		tree->setRootIndex(tree->model()->index(2, 0));
		tree->setCurrentIndex(tree->model()->index(0, 0, tree->model()->index(2, 0)));
	}
	else {
		//exceptive case.
		ctrl->setCheckState(Qt::Unchecked);
		shift->setCheckState(Qt::Unchecked);
		tree->setRootIndex(tree->model()->index(0, 0));
	}
}

TeKeySetting::TeKeySetting(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout*  hbox = new QHBoxLayout();

	QVBoxLayout* vbox = new QVBoxLayout();

	mp_list = new QTreeWidget();

	mp_list->setColumnCount(2);
	mp_list->setIndentation(1);
	mp_list->setAlternatingRowColors(true);
	mp_list->headerItem()->setText(0, tr("Key"));
	mp_list->headerItem()->setText(1, tr("Command"));
	mp_list->addTopLevelItems(createKeyTreeItem());
	mp_list->setRootIndex(mp_list->model()->index(0, 0));

	connect(mp_list, &QTreeWidget::itemActivated, [](QTreeWidgetItem* item, int /*col*/) {
		item->setData(1, Qt::DisplayRole, QVariant());
		item->setData(1, Qt::DecorationRole, QVariant()); 
		item->setData(2, Qt::DisplayRole, TeTypes::CMDID_NONE);
	});

	vbox->addWidget(mp_list);
	QHBoxLayout* cbox = new QHBoxLayout();
	QCheckBox* ctrlCheck = new QCheckBox("Ctrl");
	cbox->addWidget(ctrlCheck);
	QCheckBox*  shiftCheck = new QCheckBox("Shift");
	cbox->addWidget(shiftCheck);
	vbox->addLayout(cbox);

	connect(ctrlCheck, &QCheckBox::stateChanged, [this,ctrlCheck, shiftCheck](int /*state*/) { changeKeyTree(Qt::CTRL, ctrlCheck, shiftCheck, mp_list);	});
	connect(shiftCheck, &QCheckBox::stateChanged, [this,ctrlCheck, shiftCheck](int /*state*/) { changeKeyTree(Qt::SHIFT, ctrlCheck, shiftCheck, mp_list);	});

	hbox->addLayout(vbox);

	QTreeWidget* cmd = new QTreeWidget();

	cmd->setColumnCount(2);
	cmd->setIndentation(10);
	cmd->setAlternatingRowColors(true);
	cmd->headerItem()->setText(0, tr("Command"));
	cmd->headerItem()->setText(1, tr("Description"));

	cmd->addTopLevelItems(createCmdTreeItem());

	connect(cmd, &QTreeWidget::itemActivated, [this](QTreeWidgetItem* item, int /*col*/) {
		QTreeWidgetItem* cur = mp_list->currentItem();
		if (cur != Q_NULLPTR && (item->columnCount() == 3)) {
			cur->setData(1, Qt::DisplayRole, item->data(0, Qt::DisplayRole));
			cur->setData(1, Qt::DecorationRole, item->data(0, Qt::DecorationRole));
			cur->setData(2, Qt::DisplayRole, item->data(2, Qt::DisplayRole));
		}
	});

	hbox->addWidget(cmd);
	layout->addLayout(hbox);

	//Register OK and Cancel.
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeKeySetting::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &TeKeySetting::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeKeySetting::~TeKeySetting()
{
}

void TeKeySetting::updateSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_KEY);

	QTreeWidgetItem* rootItem = mp_list->topLevelItem(0);
	for (int i = 0; i < rootItem->childCount(); i++) {
		QTreeWidgetItem* item = rootItem->child(i);
		settings.setValue(item->data(0, Qt::DisplayRole).toString().replace("+", "_"), item->data(2, Qt::DisplayRole));
	}
	rootItem = mp_list->topLevelItem(1);
	for (int i = 0; i < rootItem->childCount(); i++) {
		QTreeWidgetItem* item = rootItem->child(i);
		settings.setValue(item->data(0, Qt::DisplayRole).toString().replace("+", "_"), item->data(2, Qt::DisplayRole));
	}
	rootItem = mp_list->topLevelItem(2);
	for (int i = 0; i < rootItem->childCount(); i++) {
		QTreeWidgetItem* item = rootItem->child(i);
		settings.setValue(item->data(0, Qt::DisplayRole).toString().replace("+", "_"), item->data(2, Qt::DisplayRole));
	}
}

void TeKeySetting::storeDefaultSettings(bool force)
{
	QSettings settings;

	if (settings.contains(SETTING_KEY)) {
		if (!force) {
			return;
		}
		settings.remove(SETTING_KEY);
	}

	settings.beginGroup(SETTING_KEY);

#define SETTING( key, default_value)  settings.setValue( QKeySequence(key).toString().replace("+", "_") , settings.value( QKeySequence(key).toString().replace("+", "_") , default_value ))

	SETTING(Qt::Key_C, TeTypes::CMDID_SYSTEM_FILE_COPY_TO);
	SETTING(Qt::Key_M, TeTypes::CMDID_SYSTEM_FILE_MOVE_TO);
	SETTING(Qt::Key_D, TeTypes::CMDID_SYSTEM_FILE_DELETE);

	SETTING(Qt::Key_A, TeTypes::CMDID_SYSTEM_EDIT_SELECT_ALL);
	SETTING(Qt::Key_Z, TeTypes::CMDID_SYSTEM_EDIT_SELECT_TOGGLE);

	SETTING(Qt::Key_V, TeTypes::CMDID_SYSTEM_TOOL_VIEW_FILE);

#undef SETTING	
	settings.endGroup();
}

QList<QTreeWidgetItem*> TeKeySetting::createKeyTreeItem()
{
	QList<int> normal, ctrl, shift;
	for (uint32_t key = Qt::Key_A; key <= Qt::Key_Z; key++) {
		normal.append(key);
		ctrl.append((int)Qt::CTRL + key);
	}
	for (uint32_t key = Qt::Key_0; key <= Qt::Key_9; key++) {
		normal.append(key);
		ctrl.append((int)Qt::CTRL + key);
	}
	for (uint32_t key = Qt::Key_F1; key <= Qt::Key_F12; key++) {
		normal.append(key);
		ctrl.append((int)Qt::CTRL + key);
		shift.append((int)Qt::SHIFT + key);
	}
	normal.append(Qt::Key_Escape);
	normal.append(Qt::Key_Backspace);
	normal.append(Qt::Key_Delete);

	QList<QTreeWidgetItem*> treeItem;
	//Normal Key
	QTreeWidgetItem* rootItem = new QTreeWidgetItem({"Normal Key"});
	rootItem->addChildren(createKeyItem(normal));
	treeItem.append(rootItem);
	//Ctrl Key
	rootItem = new QTreeWidgetItem({ "Ctrl Key" });
	rootItem->addChildren(createKeyItem(ctrl));
	treeItem.append(rootItem);
	//Shift Key
	rootItem = new QTreeWidgetItem({ "Shift Key" });
	rootItem->addChildren(createKeyItem(shift));
	treeItem.append(rootItem);

	return treeItem;
}

QList<QTreeWidgetItem*> TeKeySetting::createKeyItem(const QList<int>& list)
{
	QSettings settings;
	TeCommandFactory* factory = TeCommandFactory::factory();
	settings.beginGroup(SETTING_KEY);
	QList<QTreeWidgetItem*> result;
	for (const auto& key : list) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		QKeySequence keySeq(key);
		item->setData(0, Qt::DisplayRole, QKeySequence(key));
		TeTypes::CmdId cmdId = static_cast<TeTypes::CmdId>(settings.value(keySeq.toString().replace("+","_")).toInt());
		const TeCommandInfoBase* info = factory->commandInfo(cmdId);
		if (info != nullptr) {
			QString name = info->name();
			item->setData(1, Qt::DisplayRole, name.replace("&", ""));
			item->setData(1, Qt::DecorationRole, info->icon());
			item->setData(2, Qt::DisplayRole, info->cmdId());
		}
		else {
			item->setData(2, Qt::DisplayRole, TeTypes::CMDID_NONE);
		}
		result.append(item);
	}
	return result;
}

QList<QTreeWidgetItem*> TeKeySetting::createCmdTreeItem()
{
	QList<QTreeWidgetItem*> treeItem;
	QList<QPair<QString, TeTypes::CmdId>>  list = TeCommandFactory::groupList();

	for (auto& item : list) {
		QTreeWidgetItem* rootItem = new QTreeWidgetItem({ item.first.replace("&","") });
		rootItem->addChildren(createCmdItem(item.second));
		treeItem.append(rootItem);
	}
	return treeItem;
}

QList<QTreeWidgetItem*> TeKeySetting::createCmdItem(TeTypes::CmdId cmdGroup)
{
	QList<QTreeWidgetItem*> items;
	TeCommandFactory* factory = TeCommandFactory::factory();

	QList<const TeCommandInfoBase*>  list = factory->commandGroup(cmdGroup);
	for (auto& info : list) {
		QString name = info->name();
		QTreeWidgetItem* item = new QTreeWidgetItem({ name.replace("&",""),info->description() });
		item->setData(0, Qt::DecorationRole, info->icon());
		item->setData(2, Qt::DisplayRole, info->cmdId());
		items.append(item);
	}

	return items;
}

void TeKeySetting::accept()
{
	updateSettings();
	QDialog::accept();
}
