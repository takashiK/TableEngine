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
#include "utils/TeFileIconProvider.h"
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
#include <QScrollBar>
#include <QScopedValueRollback>
#include <QItemSelection>
#include <QTimer>
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
	mp_treeModel->setIconProvider(TeFileIconProvider::folderInstance());
	mp_listModel->setIconProvider(TeFileIconProvider::instance());

	mp_treeModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	mp_treeModel->setRootPath("");
	mp_treeSortModel = new TeFileSortProxyModel(this);
	mp_treeSortModel->setSourceModel(mp_treeModel);
	mp_treeSortModel->setDynamicFolderIcons(true);
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
			if (m_updatingPath)
				return;
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
	connectListModelSignals();

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

void TeFileFolderView::connectListModelSignals()
{
	connect(mp_listModel, &QFileSystemModel::directoryLoaded,
				[this](const QString& )
		{ if (mp_listView->currentIndex().isValid() == false) {
			QModelIndex sourceRoot = mp_listSortModel->mapToSource(mp_listView->rootIndex());
			QModelIndex index = mp_listSortModel->mapFromSource(mp_listModel->index(0, 0, sourceRoot));
			mp_listView->setCurrentIndex(index);
		}});
}

void TeFileFolderView::refreshIcons()
{
	// The tree proxy resolves icons per painted row, so repainting is enough.
	mp_treeView->viewport()->update();
}

void TeFileFolderView::refresh()
{
	const QString path = mp_listModel->rootPath();
	if (path.isEmpty()) {
		TeFolderView::refresh();
		return;
	}

	const QDir::Filters filters = mp_listModel->filter();
	const QFileSystemModel::Options options = mp_listModel->options();
	const bool readOnly = mp_listModel->isReadOnly();
	const QStringList nameFilters = mp_listModel->nameFilters();
	const bool nameFilterDisables = mp_listModel->nameFilterDisables();

	//Capture current item / multi-selection / scroll position to restore after reload.
	const QModelIndex curProxy = mp_listView->currentIndex();
	const QString curPath = curProxy.isValid()
		? mp_listModel->filePath(mp_listSortModel->mapToSource(curProxy))
		: QString();

	QStringList selectedPaths;
	if (mp_listView->selectionModel() != nullptr) {
		for (const QModelIndex& proxyIndex : mp_listView->selectionModel()->selectedIndexes()) {
			selectedPaths.append(mp_listModel->filePath(mp_listSortModel->mapToSource(proxyIndex)));
		}
	}

	const int scrollValue = mp_listView->verticalScrollBar()->value();

	QFileSystemModel* oldModel = mp_listModel;
	QFileSystemModel* newModel = new QFileSystemModel;
	newModel->setIconProvider(TeFileIconProvider::instance());
	newModel->setOptions(options);
	newModel->setReadOnly(readOnly);
	newModel->setFilter(filters);
	newModel->setNameFilters(nameFilters);
	newModel->setNameFilterDisables(nameFilterDisables);

	// A rejected root (e.g. vanished share) keeps the old model and view state untouched.
	QModelIndex rootIndex = newModel->setRootPath(path);
	if (!rootIndex.isValid()) {
		delete newModel;
		return;
	}
	mp_listModel = newModel;

	//TeFileSortProxyModel::setSourceModel() disconnects oldModel and connects newModel internally.
	mp_listSortModel->setSourceModel(newModel);
	connectListModelSignals();

	//Restore view state once newModel reports the same directory loaded; guard against
	//a stale callback firing after mp_listModel has since been replaced again.
	connect(newModel, &QFileSystemModel::directoryLoaded, this,
		[this, newModel, path, curPath, selectedPaths, scrollValue](const QString& loadedPath)
	{
		if (mp_listModel != newModel || QDir::cleanPath(loadedPath) != QDir::cleanPath(path)) {
			return;
		}

		if (!selectedPaths.isEmpty() && mp_listView->selectionModel() != nullptr) {
			QItemSelection selection;
			for (const QString& selPath : selectedPaths) {
				QModelIndex idx = mp_listSortModel->mapFromSource(newModel->index(selPath));
				if (idx.isValid())
					selection.select(idx, idx);
			}
			if (!selection.isEmpty())
				mp_listView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
		}

		if (!curPath.isEmpty()) {
			QModelIndex idx = mp_listSortModel->mapFromSource(newModel->index(curPath));
			if (idx.isValid())
				mp_listView->setCurrentIndex(idx);
		}

		mp_listView->verticalScrollBar()->setValue(scrollValue);
	}, Qt::SingleShotConnection);

	auto listSortRoot = mp_listSortModel->mapFromSource(rootIndex);
	mp_listView->setRootIndex(listSortRoot);

	delete oldModel;
}

TeFileInfo TeFileFolderView::currentFileInfo() const
{
	QModelIndex current = mp_listView->currentIndex();
	if (!current.isValid())
		return TeFileInfo();

	QFileInfo fi = mp_listModel->fileInfo(mp_listSortModel->mapToSource(current));
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
	if (pView == nullptr) {
		return;
	}

	QPoint gpos = pView->mapToGlobal(pos);
	QStringList files;
	if (collectFileContextMenuTarget(pView, pos, files)) {
		//ContextMenu for the item.
#ifdef Q_OS_LINUX
		//No native shell context menu on Linux; fall back to the
		//user-configurable popup menu. Windows always shows its native menu
		//here, so this fallback is kept Linux-only to avoid changing its
		//existing mixed-selection behaviour.
		if (!::showFilesContext(gpos.x(), gpos.y(), files)) {
			showUserContextMenu(pView == mp_treeView ? SETTING_TREEPOPUP_GROUP : SETTING_LISTPOPUP_GROUP, gpos);
		}
#else
		::showFilesContext(gpos.x(), gpos.y(), files);
#endif
	}
	else if (pView == mp_treeView) {
		//ContextMenu at no selected / ".." for treeView
		showUserContextMenu(SETTING_TREEPOPUP_GROUP, gpos);
	}
	else if (pView == mp_listView) {
		//ContextMenu at no selected / ".." for listView
		showUserContextMenu(SETTING_LISTPOPUP_GROUP, gpos);
	}
}

bool TeFileFolderView::collectFileContextMenuTarget(const QAbstractItemView* pView, const QPoint& pos, QStringList& files) const
{
	auto filePathForIndex = [this, pView](const QModelIndex& modelIndex) {
		if (pView == mp_treeView) {
			return mp_treeModel->fileInfo(mp_treeSortModel->mapToSource(modelIndex)).filePath();
		}
		if (pView == mp_listView) {
			return mp_listModel->fileInfo(mp_listSortModel->mapToSource(modelIndex)).filePath();
		}
		return QString();
	};
	auto isDotDot = [pView](const QModelIndex& modelIndex) {
		return pView->model()->data(modelIndex, Qt::DisplayRole).toString() == QStringLiteral("..");
	};

	QPersistentModelIndex index = pView->indexAt(pos);
	// ".." must never be a context-menu target; treat it like blank space so
	// the background/user context menu opens instead of a stale-selection
	// native file menu.
	if (!index.isValid() || isDotDot(index) || index != pView->currentIndex()) {
		return false;
	}

	if (pView->selectionModel()->hasSelection()) {
		QModelIndexList list = pView->selectionModel()->selectedIndexes();
		for (const auto& sindex : list) {
			if (sindex.column() != 0) {
				continue; // selectedIndexes() returns one entry per column; only row 0 is needed.
			}
			if (isDotDot(sindex)) {
				continue; // defensive: ".." must never surface even via a stale multi-selection.
			}
			const QString filePath = filePathForIndex(sindex);
			if (!filePath.isEmpty()) {
				files.append(filePath);
			}
		}
	}
	else {
		const QString filePath = filePathForIndex(index);
		if (!filePath.isEmpty()) {
			files.append(filePath);
		}
	}
	return true;
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
	trySetRootPath(path);
}

bool TeFileFolderView::trySetRootPath(const QString& path)
{
	if (!updatePath(path, QString()))
		return false;
	m_history.push({ rootPath(), currentPath()});
	return true;
}

QString TeFileFolderView::rootPath()
{
	return mp_treeModel->filePath(mp_treeSortModel->mapToSource(tree()->visualRootIndex()));
}

void TeFileFolderView::setCurrentPath(const QString & path)
{
	trySetCurrentPath(path);
}

bool TeFileFolderView::trySetCurrentPath(const QString& path)
{
	if (!updatePath(rootPath(), path))
		return false;
	m_history.push({ rootPath(), currentPath() });
	return true;
}

bool TeFileFolderView::trySetPaths(const QString& root, const QString& current)
{
	if (!updatePath(root, current))
		return false;
	m_history.push({ rootPath(), currentPath() });
	return true;
}

QString TeFileFolderView::currentPath()
{
	return mp_treeModel->filePath(mp_treeSortModel->mapToSource(tree()->currentIndex()));
}

void TeFileFolderView::moveNextPath()
{
	TeHistory::PathPair pair = m_history.next();
	// A rejected entry must not leave the history cursor advanced.
	if (pair != TeHistory::PathPair()
		&& ((!pair.second.isEmpty() && !mp_treeModel->index(pair.second).isValid())
			|| !updatePath(pair.first, pair.second)))
		m_history.previous();
}

void TeFileFolderView::movePrevPath()
{
	TeHistory::PathPair pair = m_history.previous();
	if (pair != TeHistory::PathPair()
		&& ((!pair.second.isEmpty() && !mp_treeModel->index(pair.second).isValid())
			|| !updatePath(pair.first, pair.second)))
		m_history.next();
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

bool TeFileFolderView::setListModelRoot(const QString& path, QModelIndex& rootIndex)
{
	const QString prevRoot = mp_listModel->rootPath();
	rootIndex = mp_listModel->setRootPath(path);

	// QFileSystemModel returns the previous root's index (or an invalid one) when it rejects @p path.
	const bool moved = QDir::cleanPath(mp_listModel->rootPath()) != QDir::cleanPath(prevRoot)
		|| QDir::cleanPath(path) == QDir::cleanPath(prevRoot);
	if (rootIndex.isValid() && moved
		&& QDir::cleanPath(mp_listModel->filePath(rootIndex)) == QDir::cleanPath(path))
		return true;

	mp_listModel->setRootPath(prevRoot);
	rootIndex = QModelIndex();
	return false;
}

bool TeFileFolderView::updatePath(const QString& root, const QString& current)
{
	QScopedValueRollback<bool> updatingGuard(m_updatingPath, true);

	QDir dir;
	QString absRoot = dir.absoluteFilePath(root);

	// Validate every target before touching the views so a rejected path keeps the previous state.
	const bool rootChanged = (rootPath() != absRoot);
	QModelIndex treeRootSource;
	if (rootChanged) {
		treeRootSource = mp_treeModel->index(absRoot);
		if (!treeRootSource.isValid())
			return false;
	}

	const QString prevListRoot = mp_listModel->rootPath();
	const QString prevVirtualRoot = mp_listSortModel->virtualRootPath();
	const QPersistentModelIndex prevTreeRoot = tree()->visualRootIndex();
	const QPersistentModelIndex prevTreeCurrent = tree()->currentIndex();
	const QPersistentModelIndex prevListRootIndex = list()->rootIndex();
	const QPersistentModelIndex prevListCurrent = list()->currentIndex();

	// Restores the pre-call state when a later step rejects the target after the root was committed.
	auto rollback = [&]() {
		if (!rootChanged)
			return;
		QModelIndex ignored;
		mp_listSortModel->setVirtualRootPath(prevVirtualRoot);
		setListModelRoot(prevListRoot, ignored);
		tree()->setVisualRootIndex(QModelIndex(prevTreeRoot));
		tree()->setCurrentIndex(QModelIndex(prevTreeCurrent));
		list()->setRootIndex(QModelIndex(prevListRootIndex));
		list()->setCurrentIndex(QModelIndex(prevListCurrent));
	};

	if (rootChanged) {
		//Set target path of listview to same as treeview.
		mp_listSortModel->setVirtualRootPath(absRoot);
		QModelIndex rootIndex;
		if (!setListModelRoot(absRoot, rootIndex)) {
			mp_listSortModel->setVirtualRootPath(prevVirtualRoot);
			return false;
		}

		//Set root path to treeview.
		QModelIndex index = mp_treeSortModel->mapFromSource(treeRootSource);
		tree()->setVisualRootIndex(index);
		tree()->setCurrentIndex(index);

		list()->clearSelection();
		list()->setCurrentIndex(QModelIndex());

		auto listSortRoot = mp_listSortModel->mapFromSource(rootIndex);
		list()->setRootIndex(listSortRoot);

		QModelIndex curIndex = mp_listSortModel->index(0, 0, listSortRoot);
		if (curIndex.isValid())
			list()->setCurrentIndex(curIndex);
		//invalid case : setCurrentIndex at directoryLoaded event.

	}

	if (!current.isEmpty()) {
		//Set current path to listview. and current index to treeview.
		QModelIndex prevIndex = mp_listSortModel->mapToSource(list()->rootIndex());
		QString prevPath = mp_listModel->filePath(prevIndex);
		QModelIndex index = mp_listModel->index(current);
		QModelIndex listRootIndex;
		const bool listChanged = (list()->rootIndex() != mp_listSortModel->mapFromSource(index));
		if (listChanged && !setListModelRoot(current, listRootIndex)) {
			rollback();
			return false;
		}

		index = mp_treeSortModel->mapFromSource(mp_treeModel->index(current));
		if (tree()->currentIndex() != index) {
			tree()->clearSelection();
			tree()->setCurrentIndex(index);
		}

		if (listChanged) {
			list()->clearSelection();
			list()->setCurrentIndex(QModelIndex());
			auto listSortRoot = mp_listSortModel->mapFromSource(listRootIndex);
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

	return true;
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
