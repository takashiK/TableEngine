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
#include "commands/file/TeCmdCopyTo.h"
#include "commands/file/TeCmdMoveTo.h"
#include "utils/TeFileFinder.h"
#include "utils/TeFileInfo.h"
#include "TeFileSortProxyModel.h"

#include <QLayout>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QStorageInfo>
#include <QMenu>
#include <QSettings>
#include <QStack>
#include <QMimeData>
/**
 * @file TeFileFolderView.cpp
 * @brief Implementation of TeFileFolderView.
 * @ingroup widgets
 */

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
	mp_treeSortModel = new TeFileSortProxyModel(this);
	mp_treeSortModel->setSourceModel(mp_treeModel);
	mp_treeSortModel->setShowHiddenFiles(false);
	mp_treeView->setModel(mp_treeSortModel);

	for (int i = 1; i<mp_treeView->header()->count(); i++) {
		mp_treeView->header()->setSectionHidden(i, true);
	}
	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	mp_listModel->setFilter(QDir::Drives | QDir::AllDirs | QDir::Files | QDir::NoDot );
	mp_listSortModel = new TeFileSortProxyModel(this);
	mp_listSortModel->setSourceModel(mp_listModel);
	mp_listSortModel->setPixmapSize(QSize(192,192));
	mp_listSortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	mp_listView->setModel(mp_listSortModel);
	setFileShowMode(TeTypes::FILETYPE_ALL, TeTypes::ORDER_NAME, false);
	mp_listView->setFileViewMode(TeTypes::FILEINFO_NONE, TeTypes::FILEVIEW_SMALL_ICON);
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);
//	mp_listView->setSpacing(1);
	mp_listView->setSelectionRectVisible(true);

	//mp_listView->setSelectionModel(new DebugItemSelectionModel(mp_listModel));

	//set default path
	setRootPath(QDir::rootPath());

	//connect action that work when selected path is changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged, 
		[this](const QModelIndex &proxyIndex, const QModelIndex &/*previous*/)
		{ 
			QModelIndex sourceIndex = mp_treeSortModel->mapToSource(proxyIndex);
			setCurrentPath(mp_treeModel->filePath(sourceIndex));
		 });

	// Notify the detail panel / status bar when the focused list item changes
	connect(mp_listView->selectionModel(), &QItemSelectionModel::currentChanged,
		[this](const QModelIndex&, const QModelIndex&) {
			emit currentFileChanged(currentFileInfo());
		});

	connect(mp_listView, &QListView::activated, 
		[this](const QModelIndex &proxyIndex)
	{	
		QModelIndex index = mp_listSortModel->mapToSource(proxyIndex);
		if (mp_listModel->isDir(index)){
			setCurrentPath(mp_listModel->filePath(index));
		}
		else {
			//file open by double click.
			openFile(mp_listModel->filePath(index));
		}
	});

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
			QModelIndex sourceRoot = mp_listSortModel->mapToSource(mp_listView->rootIndex());
			QModelIndex index = mp_listSortModel->mapFromSource(mp_listModel->index(0, 0, sourceRoot));
			mp_listView->setCurrentIndex(index);
		}});

	//Enable Drag & Drop
	auto dropFunc = [this](TeTypes::WidgetType type, TeFileSortProxyModel const* proxyModel, QFileSystemModel const* sourceModel, const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
		//drag & drop support for listview.
		TeTypes::CmdId cmdId = TeTypes::CMDID_NONE;
		TeCmdParam param;
		QString targetPath = sourceModel->filePath(proxyModel->mapToSource(parent));
		QStringList files;
		for (const auto& url : data->urls()) {
			files.append(url.toLocalFile());
		}

		switch(action) {
		case Qt::CopyAction:
			cmdId = TeTypes::CMDID_SYSTEM_FILE_COPY_TO;
			param.insert(TeCmdCopyTo::PARAM_TARGET_PATH, targetPath);
			param.insert(TeCmdCopyTo::PARAM_FILE_LIST, files);
			break;
		case Qt::MoveAction:
			// but not supported. because it needs to interactive action. so, we couldn't use quequed command.
			// cmdId = TeTypes::CMDID_SYSTEM_FILE_MOVE_TO;
			// param.insert(TeCmdMoveTo::PARAM_TARGET_PATH, targetPath);
			// param.insert(TeCmdMoveTo::PARAM_FILE_LIST, files);
			// break;
		default:
			return false;
		}
		execCommand(cmdId, type, nullptr, &param);
		return true;
	};

	mp_listView->setDragDropMode(QListView::DragDrop);
	mp_listView->setAcceptDrops(true);
	mp_listSortModel->setDropTarget(TeFileSortProxyModel::DropTargetFolder);
	mp_listSortModel->setDropFunction([this,dropFunc](const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
		//drag & drop support for listview.
		return dropFunc(TeTypes::WT_LISTVIEW, mp_listSortModel, mp_listModel, data, action, row, column, parent);
	});

	mp_treeView->setDragDropMode(QListView::DropOnly);
	mp_treeView->setAcceptDrops(true);
	mp_treeSortModel->setDropTarget(TeFileSortProxyModel::DropTargetFolder);
	mp_treeSortModel->setDropFunction([this,dropFunc](const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
		//drag & drop support for treeview.
		return dropFunc(TeTypes::WT_TREEVIEW, mp_treeSortModel, mp_treeModel, data, action, row, column, parent);
	});

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

TeTypes::WidgetType TeFileFolderView::getType() const
{
	return TeTypes::WT_FOLDERVIEW;
}

TeFileTreeView * TeFileFolderView::tree()
{
	return mp_treeView;
}

TeFileListView * TeFileFolderView::list()
{
	return mp_listView;
}

TeFileInfo TeFileFolderView::currentFileInfo() const
{
	QModelIndex current = mp_listView->currentIndex();
	if (!current.isValid())
		return TeFileInfo();

	QVariant var = current.data(QFileSystemModel::FileInfoRole);
	if (!var.isValid() || !var.canConvert<QFileInfo>())
		return TeFileInfo();

	QFileInfo fi = qvariant_cast<QFileInfo>(var);
	TeFileInfo info;
	info.type = fi.isDir() ? TeFileInfo::EN_DIR : TeFileInfo::EN_FILE;
	info.path = QDir::fromNativeSeparators(fi.absoluteFilePath());
	info.size = fi.isDir() ? 0 : fi.size();
	info.lastModified = fi.lastModified();
	info.permissions = static_cast<int>(fi.permissions());
	return info;
}

void TeFileFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos)
{
	if (pView != nullptr) {
		QPoint gpos = pView->mapToGlobal(pos);
		QPersistentModelIndex index = pView->indexAt(pos);
		if (index.isValid() && index == pView->currentIndex()) {
			//ContextMenu for the item
			QStringList files;
			if (pView->selectionModel()->hasSelection()) {
				QModelIndexList list = pView->selectionModel()->selectedIndexes();
				for (const auto& sindex : list) {
					QVariant var = sindex.data(QFileSystemModel::FileInfoRole);
					if (var.isValid() && var.canConvert<QFileInfo>()) {
						QFileInfo fileInfo = qvariant_cast<QFileInfo>(var);
						files.append(fileInfo.filePath());
					}
				}
			}
			else {
				QVariant var = index.data(QFileSystemModel::FileInfoRole);
				if (var.isValid() && var.canConvert<QFileInfo>()) {
					QFileInfo fileInfo = qvariant_cast<QFileInfo>(var);
					files.append(fileInfo.filePath());
				}
			}
			::showFilesContext(gpos.x(), gpos.y(), files);
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

		while (indent+1 < menus.size()) {
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
			const TeCommandInfoBase* p_info = p_factory->commandInfo(cmdId);
			if (p_info == nullptr) {
				qDebug() << "Setting File Error: Invalid command ID.";
				continue;
			}
			QAction* action = new QAction(p_info->icon(), p_info->name());
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
	updatePath(path, QString());
	m_history.push({ rootPath(), currentPath()});
}

QString TeFileFolderView::rootPath()
{
	return mp_treeModel->filePath(mp_treeSortModel->mapToSource(tree()->visualRootIndex()));
}

void TeFileFolderView::setCurrentPath(const QString & path)
{
	updatePath(rootPath(), path);
	m_history.push({ rootPath(), currentPath() });
}

QString TeFileFolderView::currentPath()
{
	return mp_treeModel->filePath(mp_treeSortModel->mapToSource(tree()->currentIndex()));
}

void TeFileFolderView::moveNextPath()
{
	TeHistory::PathPair pair = m_history.next();
	if(pair != TeHistory::PathPair())
		updatePath(pair.first, pair.second);
}

void TeFileFolderView::movePrevPath()
{
	TeHistory::PathPair pair = m_history.previous();
	if (pair != TeHistory::PathPair())
		updatePath(pair.first, pair.second);
}

void TeFileFolderView::moveParentPath() {
	QString curPath = currentPath();
	if (curPath.isEmpty() || curPath == QDir::rootPath()) {
		// Already at the root; no parent to navigate to
		return;
	}

	QDir dir(curPath);
	dir.cdUp();
	QString parentPath = dir.absolutePath();
	if(parentPath.startsWith(rootPath())) {
		setCurrentPath(parentPath);
	}
}

QStringList TeFileFolderView::getPathHistory() const
{
	return m_history.get();
}

QStringList TeFileFolderView::getPathHistoryWithRoot(const QString& root) const {
	return m_history.getWithRoot(root);
}

TeFinder* TeFileFolderView::makeFinder()
{
	if(rootPath().isEmpty() == false)
		return new TeFileFinder(rootPath());
	return nullptr;
}

void TeFileFolderView::updatePath(const QString& root, const QString& current)
{

	QDir dir;
	QString absRoot = dir.absoluteFilePath(root);
	if (rootPath() != absRoot) {
		//Set root path to treeview.
		QModelIndex index = mp_treeSortModel->mapFromSource(mp_treeModel->index(absRoot));
		tree()->setVisualRootIndex(index);
		tree()->setCurrentIndex(index);

		//Set target path of listview to same as treeview.
		list()->clearSelection();
		list()->setCurrentIndex(QModelIndex());
		mp_listSortModel->setVirtualRootPath(absRoot);
		QModelIndex rootIndex = mp_listModel->setRootPath(absRoot);

		auto listSortRoot = mp_listSortModel->mapFromSource(rootIndex);
		list()->setRootIndex(listSortRoot);

		QModelIndex curIndex = mp_listSortModel->index(0, 0, listSortRoot);
		if (curIndex.isValid())
			list()->setCurrentIndex(curIndex);
		//invalid case : setCurrentIndex at directoryLoaded event.

	}

	if (!current.isEmpty()) {
		//Set current path to listview. and current index to treeview.
		QModelIndex index = mp_treeSortModel->mapFromSource(mp_treeModel->index(current));
		if (tree()->currentIndex() != index) {
			tree()->clearSelection();
			tree()->setCurrentIndex(index);
		}

		QModelIndex prevIndex = mp_listSortModel->mapToSource(list()->rootIndex());
		QString prevPath = mp_listModel->filePath(prevIndex);
		index = mp_listModel->index(current);
		if (list()->rootIndex() != mp_listSortModel->mapFromSource(index)) {
			list()->clearSelection();
			list()->setCurrentIndex(QModelIndex());
			QModelIndex rootIndex = mp_listModel->setRootPath(current);
			auto listSortRoot = mp_listSortModel->mapFromSource(rootIndex);
			list()->setRootIndex(listSortRoot);

			if (current == mp_listModel->filePath(prevIndex.parent())) {
				list()->setCurrentIndex(mp_listSortModel->mapFromSource(mp_listModel->index(prevPath)));
			}
			else {
				QModelIndex curIndex = mp_listSortModel->index(0, 0, listSortRoot);
				if (curIndex.isValid())
					list()->setCurrentIndex(curIndex);
				//invalid case : setCurrentIndex at directoryLoaded event.
			}
		}
	}

}

void TeFileFolderView::setFileShowMode(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed)
{
	QDir::Filters filters = mp_listModel->filter();
	QDir::Filters treeFilters = mp_treeModel->filter();
	if (typeFlags.testFlag(TeTypes::FILETYPE_HIDDEN)) {
		filters |= QDir::Hidden;
		treeFilters |= QDir::Hidden;
		mp_listSortModel->setShowHiddenFiles(true);
		mp_treeSortModel->setShowHiddenFiles(true);
	}
	else {
		filters &= ~QDir::Hidden;
		treeFilters &= ~QDir::Hidden;
		mp_listSortModel->setShowHiddenFiles(false);
		mp_treeSortModel->setShowHiddenFiles(false);
	}
	if (typeFlags.testFlag(TeTypes::FILETYPE_SYSTEM)) {
		filters |= QDir::System;
		treeFilters |= QDir::System;
	}
	else {
		filters &= ~QDir::System;
		treeFilters &= ~QDir::System;
	}
	mp_listModel->setFilter(filters);
	mp_treeModel->setFilter(treeFilters);
	
	mp_listSortModel->setSortType(order);

	if(orderReversed){
		mp_listSortModel->sort(0, Qt::DescendingOrder);
	}
	else {
		mp_listSortModel->sort(0, Qt::AscendingOrder);
	}
}
