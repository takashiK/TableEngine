#include "TeArchiveFolderView.h"

#include "TeEventFilter.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"

#include <TeArchive.h>

#include <QFile>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVector>
#include <QFileIconProvider>
#include <QApplication>

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
	QAbstractItemModel* model;
	if (mp_treeView) {
		model = mp_treeView->model();
		delete mp_treeView;
		delete model;
	}
	if (mp_treeEvent) delete mp_treeEvent;

	if (mp_listView) {
		model = mp_listView->model();
		delete mp_listView;
		delete model;
	}
	if (mp_listEvent) delete mp_listEvent;
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
	TeArchive::Reader archive;
	if (!archive.open(path)) {
		return false;
	}

	return setArchive(&archive);
}

bool TeArchiveFolderView::setArchive(TeArchive::Reader * p_archive)
{
	clear();
	if (p_archive == nullptr || p_archive->path().isNull()) {
		return false;
	}

	m_mode = MODE_READ;
	m_rootPath = p_archive->path();

	for (const auto& entry : *p_archive) {
		addEntry(entry.path, entry.size, entry.type, entry.lastModifyed);
	}

	return true;
}

void TeArchiveFolderView::addEntry(const QString & path, qint64 size, TeArchive::EntryType type, const QDateTime & date)
{
	if (m_mode != MODE_WRITE) return;
	if (path.startsWith('/')) return;

	QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());

	QVector<QStringRef> paths = path.splitRef('/');

	QFileIconProvider iconProvider;

	QStandardItem* tree = mkpath(tree_model->invisibleRootItem(),paths,iconProvider);
	QStandardItem* list = mkpath(list_model->invisibleRootItem(),paths,iconProvider);

}

QString TeArchiveFolderView::modelPath(QAbstractItemModel * p_model, const QModelIndex & index)
{
	QString path;

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(p_model);
	if (model != nullptr) {
		QStandardItem* item = model->itemFromIndex(index);
		while (item != nullptr) {
			path.prepend("/" + item->text());
		}
	}

	return path;
}

QStandardItem * TeArchiveFolderView::mkpath(QStandardItem * root, const QVector<QStringRef>& paths, const QFileIconProvider & iconProvider)
{
	QStandardItem* rootItem = root;
	for (int i = 0; i < paths.count() - 1; i++) {
		int row = 0;
		for (row = 0; row < rootItem->rowCount(); row++) {
			if (rootItem->child(row)->text() == paths[i]) {
				rootItem = rootItem->child(row);
				break;
			}
		}
		if (row == rootItem->rowCount()) {
			QStandardItem* item = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder) ,paths[i].toString());
			rootItem->appendRow(item);
			rootItem = item;
#ifdef Q_OS_WIN
			item = new QStandardItem(QApplication::translate("QFileDialog", "File Folder", "Match Windows Explorer"));
#else
			item = new QStandardItem(QApplication::translate("QFileDialog", "Folder", "All other platforms"));
#endif
			rootItem->appendColumns();
		}
	}

	return rootItem;
}

void TeArchiveFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos) const
{
}

void TeArchiveFolderView::itemActivated(const QModelIndex & index)
{
}
