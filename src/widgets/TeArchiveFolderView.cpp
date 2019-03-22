#include "TeArchiveFolderView.h"

#include "TeEventFilter.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"

#include <TeArchive.h>

#include <QFile>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QHBoxLayout>

const QString TeArchiveFolderView::URI_WRITE(u8"ar_write:");
const QString TeArchiveFolderView::URI_READ(u8"ar_read:");

TeArchiveFolderView::TeArchiveFolderView(QWidget *parent)
	: TeFolderView(parent)
{
	m_mode = MODE_WRITE;

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


	mp_treeView->setModel(new QStandardItemModel);

	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	mp_listView->setModel(new QStandardItemModel);
	mp_listView->setViewMode(QListView::ListMode);
	mp_listView->setWrapping(true);
	mp_listView->setResizeMode(QListView::Adjust);
	mp_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);

	//Initialize as write mode.
	clear();

	//Hide headers of treeView.
//	for (int i = 1; i < mp_treeView->header()->count(); i++) {
//		mp_treeView->header()->setSectionHidden(i, true);
//	}

	//Synchronize views when current directory changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		[this](const QModelIndex &current, const QModelIndex &previous)
	{ setCurrentPath(modelPath(mp_treeView->model(),current)); });

	connect(mp_listView, &QListView::activated, this, &TeArchiveFolderView::itemActivated);

	//connect to custom context menu.
	connect(mp_treeView, &QTreeView::customContextMenuRequested,
		[this](const QPoint& pos)
	{ showContextMenu(mp_treeView, pos); });

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

TeArchiveFolderView::~TeArchiveFolderView()
{
}

TeFileTreeView * TeArchiveFolderView::tree()
{
	return mp_treeView;
}

TeFileListView * TeArchiveFolderView::list()
{
	return mp_listView;
}

void TeArchiveFolderView::setRootPath(const QString & path)
{
	clear();
	QString filePath;
	if (path.isNull()) {
		//Archive Write Mode
	}
	else if (path.startsWith(URI_WRITE)) {
		//Archive Write Mode
		//export path is decided in "archive" function.
		//so path is rejected in this function.
	}
	else if (path.startsWith(URI_READ)) {
		//Archive Read from URI
		filePath = path.mid(URI_READ.size());
	}
	else {
		//Arhive Read from path
		filePath = path;
	}

	if (!filePath.isNull()) {
		setArchive(filePath);
	}
}

QString TeArchiveFolderView::rootPath()
{
	if (m_mode == MODE_READ) {
		return URI_READ + m_rootPath;
	}
	return URI_WRITE;
}

void TeArchiveFolderView::setCurrentPath(const QString & path)
{
}

QString TeArchiveFolderView::currentPath()
{
	return QString();
}

void TeArchiveFolderView::clear()
{
}

bool TeArchiveFolderView::setArchive(const QString & path)
{
	return false;
}

bool TeArchiveFolderView::setArchive(TeArchive::Reader * p_archive)
{
	return false;
}

QString TeArchiveFolderView::modelPath(QAbstractItemModel * p_model, const QModelIndex & index)
{
	return QString();
}

void TeArchiveFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos) const
{
}

void TeArchiveFolderView::itemActivated(const QModelIndex & index)
{
}
