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
#include <QIcon>
#include <QDir>

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

	//Synchronize views when current directory changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		[this](const QModelIndex &current, const QModelIndex &/*previous*/)
	{ setCurrentPath(indexToPath(current)); });

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
		mp_treeView = nullptr;
	}
	if (mp_treeEvent) {
		delete mp_treeEvent;
		mp_treeEvent = nullptr;
	}

	if (mp_listView) {
		model = mp_listView->model();
		delete mp_listView;
		delete model;
		mp_listView = nullptr;
	}
	if (mp_listEvent) {
		delete mp_listEvent;
		mp_listEvent = nullptr;
	}
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
	if (path.isEmpty()) {
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

	if (!filePath.isEmpty()) {
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
	QString cur = currentPath();
	if (currentPath() != path) {
		QString target = QDir::cleanPath(path);

		QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
		QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());

		QStandardItem* entry = findPath(tree_model->item(0), target);
		if (entry != nullptr) {
			mp_treeView->setCurrentIndex(tree_model->indexFromItem(entry));
			mp_listView->clearSelection();
			entry = findPath(list_model->item(0), target);
			mp_listView->setRootIndex(list_model->indexFromItem(entry));
		}
		else {
			mp_treeView->setCurrentIndex(tree_model->index(0, 0));
			mp_listView->setRootIndex(list_model->index(0, 0));
		}
	}
}

QString TeArchiveFolderView::currentPath()
{
	return indexToPath(mp_listView->rootIndex());
}

void TeArchiveFolderView::clear()
{
	QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());
	tree_model->clear();
	list_model->clear();

	m_rootPath.clear();

	m_mode = MODE_WRITE;
	tree_model->appendRow(createRootEntry());
	list_model->appendRow(createRootEntry());

	mp_listView->setRootIndex(list_model->index(0,0));

	//Hide headers of treeView.
	for (int i = 1; i < mp_treeView->header()->count(); i++) {
		mp_treeView->header()->setSectionHidden(i, true);
	}
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
	if (p_archive == nullptr || p_archive->path().isEmpty()) {
		return false;
	}

	m_mode = MODE_READ;
	m_rootPath = p_archive->path();
	QFileInfo info(m_rootPath);

	QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());
	
	tree_model->item(0)->setData(info.fileName(), Qt::DisplayRole);
	list_model->item(0)->setData(info.fileName(), Qt::DisplayRole);

	for (const auto& entry : *p_archive) {
		switch (entry.type) {
		case TeArchive::EN_DIR:
			internalAddDirEntry(entry.path);
			break;
		case TeArchive::EN_FILE:
			internalAddEntry(entry.path, entry.size, entry.lastModifyed, entry.src);
			break;
		default:
			break;
		}
	}

	return true;
}

TeArchive::Writer * TeArchiveFolderView::archive()
{
	if (m_mode != MODE_WRITE) return nullptr;
	QStandardItemModel * model = qobject_cast<QStandardItemModel*>(mp_listView->model());
	if (!model->item(0)->hasChildren()) return nullptr;

	TeArchive::Writer* writer = new TeArchive::Writer();

	buildArchiveEntry(writer, model->item(0));

	return writer;
}

bool TeArchiveFolderView::archive(const QString & path, TeArchive::ArchiveType type)
{
	TeArchive::Writer* writer = archive();
	if (writer == nullptr) return false;

	bool result = writer->archive(path, type);
	delete writer;

	return result;
}

void TeArchiveFolderView::addEntry(const QString & path, qint64 size, const QDateTime & lastModified, const QString& src)
{
	if (m_mode != MODE_WRITE) return;
	internalAddEntry(path, size, lastModified, src);
}

void TeArchiveFolderView::internalAddEntry(const QString & path, qint64 size, const QDateTime & lastModified, const QString& src)
{
	if (path.startsWith('/')) return;

	QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());

	QVector<QStringRef> paths = QDir::cleanPath(path).splitRef('/');

	QFileIconProvider iconProvider;

	mkpath(iconProvider,tree_model->item(0),paths,false);
	QStandardItem* list = mkpath(iconProvider,list_model->item(0),paths,true);

	if (list != nullptr && findChild(list, paths.last()) == nullptr) {
		list->appendRow(createFileEntry(iconProvider, paths.last().toString(), size, lastModified, src));
	}
}

void TeArchiveFolderView::addDirEntry(const QString & path)
{
	if (m_mode != MODE_WRITE) return;
	internalAddDirEntry(path);
}

void TeArchiveFolderView::internalAddDirEntry(const QString & path)
{
	if (path.startsWith('/')) return;

	QString destPath;
	if (path.endsWith('/')) {
		destPath = QDir::cleanPath(path);
	}
	else {
		destPath = QDir::cleanPath(path) + "/";
	}

	QStandardItemModel* tree_model = qobject_cast<QStandardItemModel*>(mp_treeView->model());
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());

	QVector<QStringRef> paths = destPath.splitRef('/');

	QFileIconProvider iconProvider;
	mkpath(iconProvider, tree_model->item(0), paths,false);
	mkpath(iconProvider, list_model->item(0), paths,true);
}

QString TeArchiveFolderView::indexToPath(const QModelIndex & index)
{
	QString path;

	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(index.model());
	if (model != nullptr) {
		QStandardItem* item = model->itemFromIndex(index);
		QStandardItem* root = model->item(0);
		while (item != nullptr && item != root) {
			path.prepend("/" + item->text());
			item = item->parent();
		}
	}

	return path.mid(1);
}

QStandardItem * TeArchiveFolderView::findPath(QStandardItem* root, const QString & path)
{
	QStandardItem* entry = root;
	QVector<QStringRef> paths = path.splitRef('/');

	for (int i = 0; i < paths.count(); i++) {
		entry = findChild(entry, paths[i]);
		if (entry == nullptr) {
			break;
		}
	}

	return entry;
}

QStandardItem * TeArchiveFolderView::mkpath(const QFileIconProvider & iconProvider, QStandardItem * root, const QVector<QStringRef>& paths, bool bParentEntry)
{
	QStandardItem* parent = root;
	for (int i = 0; i < paths.count() - 1; i++) {
		QStandardItem* child = findChild(parent,paths[i]);
		if (child != nullptr) {
			parent = child;
		}
		else{
			QList<QStandardItem*> entries = createDirEntry(iconProvider, paths[i].toString());
			parent->appendRow(entries);
			parent = entries[0];
			if (bParentEntry == true) {
				parent->appendRow(createParentEntry());
			}
		}
	}

	return parent;
}

QStandardItem * TeArchiveFolderView::findChild(const QStandardItem * parent, const QString & name)
{
	if (parent == nullptr) return nullptr;

	QStandardItem* child = nullptr;
	for (int row = 0; row < parent->rowCount(); row++) {
		if (parent->child(row)->text() == name) {
			child = parent->child(row);
			break;
		}
	}
	return child;
}

QStandardItem * TeArchiveFolderView::findChild(const QStandardItem * parent, const QStringRef & name)
{
	if (parent == nullptr) return nullptr;

	QStandardItem* child = nullptr;
	for (int row = 0; row < parent->rowCount(); row++) {
		if (parent->child(row)->text() == name) {
			child = parent->child(row);
			break;
		}
	}
	return child;
}

QList<QStandardItem*> TeArchiveFolderView::createRootEntry()
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new QStandardItem(QIcon(":/TableEngine/archiveBox.png"), tr("Archive"));
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new QStandardItem("");
	entry->setEditable(false);
	entries.append(entry);

	// Type
	entry = new QStandardItem("");
	entry->setData(TeArchive::EN_NONE);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new QStandardItem("");
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createParentEntry()
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new QStandardItem("..");
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new QStandardItem("");
	entry->setEditable(false);
	entries.append(entry);

	// Type
	entry = new QStandardItem("");
	entry->setData(TeArchive::EN_PARENT);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new QStandardItem("");
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createDirEntry(const QFileIconProvider & iconProvider, const QString & name)
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), name);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new QStandardItem(""); //directory entry doesn't have "Size". 
	entry->setEditable(false);
	entries.append(entry);

	// Type
#ifdef Q_OS_WIN
	entry = new QStandardItem(QApplication::translate("QFileDialog", "File Folder", "Match Windows Explorer"));
#else
	entry = new QStandardItem(QApplication::translate("QFileDialog", "Folder", "All other platforms"));
#endif
	entry->setData(TeArchive::EN_DIR);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new QStandardItem(""); //directory entry doesn't have "lastModified Date".
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createFileEntry(const QFileIconProvider & iconProvider, const QString & name, qint64 size, const QDateTime & lastModified, const QString& src)
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new QStandardItem(iconProvider.icon(QFileIconProvider::File), name);
	entry->setData(src);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new QStandardItem(QLocale::system().formattedDataSize(size));
	entry->setData(size);
	entry->setEditable(false);
	entries.append(entry);

	// Type
	int index = name.lastIndexOf('.');
	if (index >= 0 && index+1 < name.count()) {
		//: %1 is a file name suffix, for example txt
		entry = new QStandardItem(QApplication::translate("QFileDialog", "%1 File").arg(name.midRef(index+1)));
	}
	else {
		entry = new QStandardItem(QApplication::translate("QFileDialog", "File"));
	}

	entry->setData(TeArchive::EN_FILE);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new QStandardItem(lastModified.toString(Qt::SystemLocaleDate));
	entry->setData(lastModified);
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

void TeArchiveFolderView::buildArchiveEntry(TeArchive::Writer * writer, QStandardItem * rootItem)
{
	for (int i = 0; i < rootItem->rowCount(); i++) {
		QStandardItem* item = rootItem->child(i);
		
		if (rootItem->child(i, COL_TYPE)->data().toInt() == TeArchive::EN_FILE) {
			QString dstPath = indexToPath(item->index());
			QString srcPath = item->data().toString();
			if (!dstPath.isEmpty() && !srcPath.isEmpty()) {
				writer->addEntry(srcPath, dstPath);
			}
		}

		if (item->hasChildren()) {
			buildArchiveEntry(writer, item);
		}
	}
}

void TeArchiveFolderView::showContextMenu(const QAbstractItemView * /*pView*/, const QPoint & /*pos*/) const
{
	
}

void TeArchiveFolderView::itemActivated(const QModelIndex & index)
{
	QStandardItemModel* list_model = qobject_cast<QStandardItemModel*>(mp_listView->model());
	int type = list_model->index(index.row(), 2, index.parent()).data(Qt::UserRole + 1).toInt();
	if (type == TeArchive::EN_DIR || type == TeArchive::EN_PARENT) {
		setCurrentPath(indexToPath(index));
	}
}
