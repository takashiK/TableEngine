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
#include "TeFileFolderView.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"
#include "TeEventFilter.h"
#include "platform/platform_util.h"

#include <QLayout>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QStorageInfo>

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
	mp_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);

	//デフォルトパスの設定
	setRootPath(QDir::rootPath());

	//ディレクトリ変更時の同期動作設定
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged, 
		[this](const QModelIndex &current, const QModelIndex &previous)
		{ setCurrentPath(mp_treeModel->filePath(current)); });

	connect(mp_listView, &QListView::activated, 
		[this](const QModelIndex &index)
	{	if (mp_listModel->isDir(index)){
			setCurrentPath(mp_listModel->filePath(index));
		}
		else {
			//ダブルクリック時のファイルを開く操作
			openFile(mp_listModel->filePath(index));
		}});

	//Windows版コンテキストメニュー設定
	connect(mp_treeView, &QTreeView::customContextMenuRequested,
		[this](const QPoint& pos)
		{ showContextMenu(mp_treeView,pos); });

	connect(mp_listView, &QListView::customContextMenuRequested,
		[this](const QPoint& pos)
	{ showContextMenu(mp_listView, pos); });

	mp_treeEvent = new TeEventFilter();
	mp_treeEvent->setType(TeTypes::WT_TREEVIEW);
	mp_treeEvent->setDispatcher(this);
	mp_treeView->installEventFilter(mp_treeEvent);

	mp_listEvent = new TeEventFilter();
	mp_listEvent->setType(TeTypes::WT_LISTVIEW);
	mp_listEvent->setDispatcher(this);
	mp_listView->installEventFilter(mp_listEvent);


	QHBoxLayout* layout = new QHBoxLayout();
	layout->setMargin(0);
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

void TeFileFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos) const
{
	if (pView != nullptr) {
		QPersistentModelIndex index = pView->indexAt(pos);
		if (index.isValid() && index == pView->currentIndex()) {
			QFileSystemModel* fmodel = qobject_cast<QFileSystemModel*>(pView->model());
			QPoint gpos = pView->mapToGlobal(pos);
			::showFileContext(gpos.x(), gpos.y(), fmodel->filePath(index));
		}
	}
}

void TeFileFolderView::setRootPath(const QString & path)
{
	QString rpath = rootPath();
	if (rootPath() != path) {
		//TreeViewのルートを設定
		QModelIndex index = mp_treeModel->index(path);
		tree()->setVisualRootIndex(index);
		tree()->setCurrentIndex(index);

		//ListViewのルートをTreeViewのルートに設定
		list()->setRootIndex(mp_listModel->setRootPath(path));

		rpath = rootPath();
	}
}

QString TeFileFolderView::rootPath()
{
	return mp_treeModel->filePath(tree()->visualRootIndex());
}

void TeFileFolderView::setCurrentPath(const QString & path)
{
	//指定されたカレントパスを設定
	QModelIndex index = mp_treeModel->index(path);
	if (tree()->currentIndex() != index) {
		tree()->clearSelection();
		tree()->setCurrentIndex(index);
	}

	index = mp_listModel->index(path);
	if (list()->rootIndex() != index) {
		list()->clearSelection();
		list()->setRootIndex(mp_listModel->setRootPath(path));
	}
}

QString TeFileFolderView::currentPath()
{
	return mp_treeModel->filePath(tree()->currentIndex());
}
