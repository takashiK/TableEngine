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

namespace {

bool isVariantLessThan(const QVariant& left, const QVariant& right,
	Qt::CaseSensitivity cs, bool isLocaleAware)
{
	if (left.userType() == QMetaType::UnknownType)
		return false;
	if (right.userType() == QMetaType::UnknownType)
		return true;
	switch (left.userType()) {
	case QMetaType::Int:
		return left.toInt() < right.toInt();
	case QMetaType::UInt:
		return left.toUInt() < right.toUInt();
	case QMetaType::LongLong:
		return left.toLongLong() < right.toLongLong();
	case QMetaType::ULongLong:
		return left.toULongLong() < right.toULongLong();
	case QMetaType::Float:
		return left.toFloat() < right.toFloat();
	case QMetaType::Double:
		return left.toDouble() < right.toDouble();
	case QMetaType::QChar:
		return left.toChar() < right.toChar();
	case QMetaType::QDate:
		return left.toDate() < right.toDate();
	case QMetaType::QTime:
		return left.toTime() < right.toTime();
	case QMetaType::QDateTime:
		return left.toDateTime() < right.toDateTime();
	case QMetaType::QString:
	default:
		if (isLocaleAware)
			return left.toString().localeAwareCompare(right.toString()) < 0;
		else
			return left.toString().compare(right.toString(), cs) < 0;
	}
}


class TeStandardFileItem : public QStandardItem {
public:
	TeStandardFileItem() : QStandardItem() {}
	TeStandardFileItem(const QString& text) : QStandardItem(text) {}
	TeStandardFileItem(const QIcon& icon, const QString& text) : QStandardItem(icon, text) {}

	virtual bool operator<(const QStandardItem& other) const {
		int t = data(Qt::UserRole + 2).toInt();
		int o = other.data(Qt::UserRole + 2).toInt();
		if (t != o){
			return t>o;
		}
		const int role = model() ? model()->sortRole() : Qt::DisplayRole;
		const QVariant l = data(role), r = other.data(role);
		return isVariantLessThan(l, r,Qt::CaseInsensitive,false);
	}
};

}


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
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
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
	layout->setContentsMargins(0,0,0,0);
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

TeTypes::WidgetType TeArchiveFolderView::getType() const
{
	return TeTypes::WT_ARCHIVEVIEW;
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
		return m_rootPath;
	}
	return QString();
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
	QDir dir;
	QString absPath = dir.absoluteFilePath(path);
	if (!archive.open(absPath)) {
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

	tree_model->sort(0);
	list_model->sort(0);

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

	QStringList paths = QDir::cleanPath(path).split('/');

	QFileIconProvider iconProvider;

	mkpath(iconProvider,tree_model->item(0),paths,false);
	QStandardItem* list = mkpath(iconProvider,list_model->item(0),paths,true);

	if (list != nullptr && findChild(list, paths.last()) == nullptr) {
		list->appendRow(createFileEntry(iconProvider, paths.last(), size, lastModified, src));
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

	QStringList paths = destPath.split('/');

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
	QStringList paths = path.split('/');

	for (int i = 0; i < paths.count(); i++) {
		entry = findChild(entry, paths[i]);
		if (entry == nullptr) {
			break;
		}
	}

	return entry;
}

QStandardItem * TeArchiveFolderView::mkpath(const QFileIconProvider & iconProvider, QStandardItem * root, const QStringList& paths, bool bParentEntry)
{
	QStandardItem* parent = root;
	for (int i = 0; i < paths.count() - 1; i++) {
		QStandardItem* child = findChild(parent,paths[i]);
		if (child != nullptr) {
			parent = child;
		}
		else{
			QList<QStandardItem*> entries = createDirEntry(iconProvider, paths[i]);
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

QList<QStandardItem*> TeArchiveFolderView::createRootEntry()
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new TeStandardFileItem(QIcon(":/TableEngine/archiveBox.png"), tr("Archive"));
	entry->setData(TeArchive::EN_NONE, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_NONE, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Type
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_NONE, Qt::UserRole + 2);
	entry->setData(TeArchive::EN_NONE);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_NONE, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createParentEntry()
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new TeStandardFileItem("..");
	entry->setData(TeArchive::EN_PARENT, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_PARENT, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Type
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_PARENT, Qt::UserRole + 2);
	entry->setData(TeArchive::EN_PARENT);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem("");
	entry->setData(TeArchive::EN_PARENT, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createDirEntry(const QFileIconProvider & iconProvider, const QString & name)
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new TeStandardFileItem(iconProvider.icon(QFileIconProvider::Folder), name);
	entry->setData(TeArchive::EN_DIR, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem(""); //directory entry doesn't have "Size". 
	entry->setData(TeArchive::EN_DIR, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	// Type
#ifdef Q_OS_WIN
	entry = new TeStandardFileItem(QApplication::translate("QFileDialog", "File Folder", "Match Windows Explorer"));
#else
	entry = new TeStandardFileItem(QApplication::translate("QFileDialog", "Folder", "All other platforms"));
#endif
	entry->setData(TeArchive::EN_DIR, Qt::UserRole + 2);
	entry->setData(TeArchive::EN_DIR);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem(""); //directory entry doesn't have "lastModified Date".
	entry->setData(TeArchive::EN_DIR, Qt::UserRole + 2);
	entry->setEditable(false);
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createFileEntry(const QFileIconProvider & iconProvider, const QString & name, qint64 size, const QDateTime & lastModified, const QString& src)
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new TeStandardFileItem(iconProvider.icon(QFileIconProvider::File), name);
	entry->setData(TeArchive::EN_FILE, Qt::UserRole + 2);
	entry->setData(src);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem(QLocale::system().formattedDataSize(size));
	entry->setData(TeArchive::EN_FILE, Qt::UserRole + 2);
	entry->setData(size);
	entry->setEditable(false);
	entries.append(entry);

	// Type
	int index = name.lastIndexOf('.');
	if (index >= 0 && index+1 < name.count()) {
		//: %1 is a file name suffix, for example txt
		entry = new TeStandardFileItem(QApplication::translate("QFileDialog", "%1 File").arg(name.mid(index+1)));
	}
	else {
		entry = new TeStandardFileItem(QApplication::translate("QFileDialog", "File"));
	}

	entry->setData(TeArchive::EN_FILE, Qt::UserRole + 2);
	entry->setData(TeArchive::EN_FILE);
	entry->setEditable(false);
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem(lastModified.toString(Qt::ISODate));
	entry->setData(TeArchive::EN_FILE, Qt::UserRole + 2);
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
