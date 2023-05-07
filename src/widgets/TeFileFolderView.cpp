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

#include "TeFileFolderView.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"
#include "TeEventFilter.h"
#include "platform/platform_util.h"
#include "TeSettings.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"

#include <QLayout>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QStorageInfo>
#include <QMenu>
#include <QSettings>
#include <QStack>
#include <QDebug>

TeFileFolderView::TeFileFolderView(QWidget *parent)
	: TeFolderView(parent)
{

	mp_treeView = new TeFileTreeView;
	mp_treeView->setFolderView(this);
	mp_listView = new TeFileListView;
	mp_listView->setFolderView(this);

	QSizePolicy treePolicy = mp_treeView->sizePolicy();
	treePolicy.setHorizontalStretch(1);
	mp_treeView->setSizePolicy(treePolicy);

	QSizePolicy listPolicy = mp_listView->sizePolicy();
	listPolicy.setHorizontalStretch(2);
	mp_listView->setSizePolicy(listPolicy);

	mp_treeModel = new QFileSystemModel;
	mp_listModel = new QFileSystemModel;

	mp_treeModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	mp_treeModel->setRootPath("");
	mp_treeView->setModel(mp_treeModel);

	for (int i = 1; i<mp_treeView->header()->count(); i++) {
		mp_treeView->header()->setSectionHidden(i, true);
	}
	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	mp_listModel->setFilter(QDir::Drives | QDir::AllDirs | QDir::Files | QDir::NoDot );
	mp_listView->setModel(mp_listModel);
	mp_listView->setViewMode(QListView::ListMode);
	mp_listView->setWrapping(true);
	mp_listView->setResizeMode(QListView::Adjust);
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);
	mp_listView->setSpacing(2);
	mp_listView->setSelectionRectVisible(true);

	//set default path
	setRootPath(QDir::rootPath());

	//connect action that work when selected path is changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged, 
		[this](const QModelIndex &current, const QModelIndex &/*previous*/)
		{ setCurrentPath(mp_treeModel->filePath(current)); });

	connect(mp_listView, &QListView::activated, 
		[this](const QModelIndex &index)
	{	if (mp_listModel->isDir(index)){
			setCurrentPath(mp_listModel->filePath(index));
		}
		else {
			//file open by double click.
			openFile(mp_listModel->filePath(index));
		}});

	//set platform native context menu.
	connect(mp_treeView, &QTreeView::customContextMenuRequested,
		[this](const QPoint& pos)
		{ showContextMenu(mp_treeView,pos); });

	connect(mp_listView, &QListView::customContextMenuRequested,
		[this](const QPoint& pos)
	{ showContextMenu(mp_listView, pos); });

	//SetCurrentIndex to ListView if its currentIndex is invalid when listModel is updated.
	connect(mp_listModel, &QFileSystemModel::directoryLoaded,
				[this](const QString& )
		{ if (mp_listView->currentIndex().isValid() == false) {
			mp_listView->setCurrentIndex(mp_listModel->index(0,0,mp_listView->rootIndex()));
		}});

	//Enable Drag & Drop
	mp_listView->setDragDropMode(QListView::DragDrop);

	//Install TeDispatcher.
	mp_treeEvent = new TeEventFilter();
	mp_treeEvent->setType(TeTypes::WT_TREEVIEW);
	mp_treeEvent->setDispatcher(this);
	mp_treeView->installEventFilter(mp_treeEvent);

	mp_listEvent = new TeEventFilter();
	mp_listEvent->setType(TeTypes::WT_LISTVIEW);
	mp_listEvent->setDispatcher(this);
	mp_listView->installEventFilter(mp_listEvent);


	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);
	layout->addWidget(mp_listView);
}

TeFileFolderView::~TeFileFolderView()
{
	delete mp_treeView;
	delete mp_treeModel;
	delete mp_treeEvent;

	delete mp_listView;
	delete mp_listModel;
	delete mp_listEvent;
}

TeFileTreeView * TeFileFolderView::tree()
{
	return mp_treeView;
}

TeFileListView * TeFileFolderView::list()
{
	return mp_listView;
}

void TeFileFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos)
{
	if (pView != nullptr) {
		QPoint gpos = pView->mapToGlobal(pos);
		QPersistentModelIndex index = pView->indexAt(pos);
		if (index.isValid() && index == pView->currentIndex()) {
			//ContextMenu for the item
			QFileSystemModel* fmodel = qobject_cast<QFileSystemModel*>(pView->model());
			::showFileContext(gpos.x(), gpos.y(), fmodel->filePath(index));
		}
		else if(pView == mp_treeView){
			//ContextMenu at no selected for treeView
			showUserContextMenu(SETTING_TREEPOPUP_GROUP, gpos);

		}
		else if (pView == mp_listView) {
			//ContextMenu at no selected for listView
			showUserContextMenu(SETTING_LISTPOPUP_GROUP, gpos);

		}
	}
}

void TeFileFolderView::showUserContextMenu(const QString& menuName, const QPoint& pos)
{
	// update menu
	QSettings settings;
	TeCommandFactory* p_factory = TeCommandFactory::factory();

	QMenu menu;

	settings.beginGroup(SETTING_MENU);
	settings.beginGroup(menuName);
	QStack<QMenu*> menus;
	menus.push(&menu);

	for (const auto& key : settings.childKeys()) {
		QStringList values = settings.value(key).toString().split(',');
		int indent = values[0].toInt();
		TeTypes::CmdId cmdId = static_cast<TeTypes::CmdId>(values[2].toInt());
		QString name = values[1];

		if (indent < 0) {
			qDebug() << "Setting File Error: Invalid menu indent.";
			continue;
		}

		while (indent+1 < menus.count()) {
			menus.pop();
		}

		if (cmdId == TeTypes::CMDID_SPECIAL_FOLDER) {
			//Create Sub menu folder
			menus.push(menus.top()->addMenu(name));
		}
		else if (cmdId == TeTypes::CMDID_SPECIAL_SEPARATOR) {
			//add Separator
			menus.top()->addSeparator();
		}
		else {
			//Create Menu Item
			TeCommandInfoBase* p_info = p_factory->commandInfo(cmdId);
			QAction* action = new QAction(p_info->icon(), p_info->name());
			action->setEnabled(p_info->isAvailable());
			connect(action, &QAction::triggered, [this, cmdId](bool /*checked*/) { emit requestCommand(cmdId, TeTypes::WT_NONE, nullptr, nullptr); });
			menus.top()->addAction(action);
		}
	}
	settings.endGroup();
	settings.endGroup();

	menu.exec(pos);
}

void TeFileFolderView::setRootPath(const QString & path)
{
	if (rootPath() != path) {
		//Set root path to treeview.
		QModelIndex index = mp_treeModel->index(path);
		tree()->setVisualRootIndex(index);
		tree()->setCurrentIndex(index);

		//Set target path of listview to same as treeview.
		list()->clearSelection();
		list()->setCurrentIndex(QModelIndex());
		QModelIndex rootIndex = mp_listModel->setRootPath(path);
		list()->setRootIndex(rootIndex);

		QModelIndex curIndex = mp_listModel->index(0, 0, rootIndex);
		if (curIndex.isValid())
			list()->setCurrentIndex(curIndex);
		//invalid case : setCurrentIndex at directoryLoaded event.

	}
}

QString TeFileFolderView::rootPath()
{
	return mp_treeModel->filePath(tree()->visualRootIndex());
}

void TeFileFolderView::setCurrentPath(const QString & path)
{
	//Set current path to listview. and current index to treeview.
	QModelIndex index = mp_treeModel->index(path);
	if (tree()->currentIndex() != index) {
		tree()->clearSelection();
		tree()->setCurrentIndex(index);
	}

	QModelIndex prevIndex = list()->rootIndex();
	QString prevPath = mp_listModel->filePath(list()->rootIndex());
	index = mp_listModel->index(path);
	if (list()->rootIndex() != index) {
		list()->clearSelection();
		list()->setCurrentIndex(QModelIndex());
		QModelIndex rootIndex = mp_listModel->setRootPath(path);
		list()->setRootIndex(rootIndex);

		if(path == mp_listModel->filePath(prevIndex.parent())){
			list()->setCurrentIndex(mp_listModel->index(prevPath));
		}
		else {
			QModelIndex curIndex = mp_listModel->index(0, 0, rootIndex);
			if (curIndex.isValid())
				list()->setCurrentIndex(curIndex);
			//invalid case : setCurrentIndex at directoryLoaded event.
		}
	}
}

QString TeFileFolderView::currentPath()
{
	return mp_treeModel->filePath(tree()->currentIndex());
}
