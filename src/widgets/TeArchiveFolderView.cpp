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
#include "TeFileSortProxyModel.h"
#include "TeSettings.h"
#include "utils/TeArchiveFinder.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"
#include "dialogs/TePasswordDialog.h"

#include "utils/TeArchive.h"

#include <QFile>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVector>
#include <QFileIconProvider>
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QMenu>
#include <QSettings>
#include <QStack>
#include <QProgressDialog>
#include <QTemporaryDir>
#include <QMessageBox>
#include <QDirIterator>
#include <QDateTime>
/**
 * @file TeArchiveFolderView.cpp
 * @brief Implementation of TeArchiveFolderView.
 * @ingroup widgets
 */

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
		int t = data(TeFileInfo::ROLE_TYPE).toInt();
		int o = other.data(TeFileInfo::ROLE_TYPE).toInt();
		if (t != o){
			return t>o;
		}
		const int role = model() ? model()->sortRole() : Qt::DisplayRole;
		const QVariant l = data(role), r = other.data(role);
		return isVariantLessThan(l, r, Qt::CaseInsensitive, true);
	}
};

}


TeArchiveFolderView::TeArchiveFolderView(QWidget *parent)
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


	mp_treeProxy = new TeFileSortProxyModel(this);
	mp_treeModel = new QStandardItemModel(mp_treeProxy);
	mp_treeProxy->setSourceModel(mp_treeModel);
	mp_treeProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
	mp_treeView->setModel(mp_treeProxy);

	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	// Archive entries are backed by QStandardItemModel whose items are editable
	// by default. Disable inline editing so a left double-click emits activated
	// (matching TeFileFolderView, which uses a read-only QFileSystemModel).
	mp_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	mp_listProxy = new TeFileSortProxyModel(this);
	mp_listModel = new QStandardItemModel(mp_listProxy);
	mp_listProxy->setSourceModel(mp_listModel);
	mp_listProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
	mp_listView->setModel(mp_listProxy);
	// Configure the list delegate (size/date detail columns) and list layout
	// in one call, mirroring TeFileFolderView's list view configuration.
	mp_listView->setFileViewMode(TeTypes::FILEINFO_SIZE | TeTypes::FILEINFO_MODIFIED,
		TeTypes::FILEVIEW_SMALL_ICON);
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
	mp_listView->setSelectionRectVisible(true);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);
	mp_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// Establish an ascending, column-0 sort so subsequent inserts stay ordered
	// (mirrors TeFileFolderView's proxy configuration).
	mp_treeProxy->sort(0, Qt::AscendingOrder);
	mp_listProxy->sort(0, Qt::AscendingOrder);

	//Initialize as write mode.
	clear();

	//Synchronize views when current directory changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		[this](const QModelIndex &current, const QModelIndex &/*previous*/)
	{ setCurrentPath(indexToPath(mp_treeProxy->mapToSource(current))); });

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
	if (mp_reader) {
		delete mp_reader;
		mp_reader = nullptr;
	}
	if (mp_tempDir) {
		delete mp_tempDir;
		mp_tempDir = nullptr;
	}

	// The proxies are QObject children of this view and each source model is a
	// child of its proxy, so both are destroyed automatically with this object.
	// The views do not own their models, so delete only the views here.
	if (mp_treeView) {
		delete mp_treeView;
		mp_treeView = nullptr;
	}
	if (mp_treeEvent) {
		delete mp_treeEvent;
		mp_treeEvent = nullptr;
	}

	if (mp_listView) {
		delete mp_listView;
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
	if (path.startsWith(URI_READ)) {
		m_mode = MODE_READONLY;
		setArchive(path.mid(URI_READ.size()));
	}
	else if (path.startsWith(URI_WRITE)) {
		m_mode = MODE_WRITABLE;
		clear();
		m_rootPath = path.mid(URI_WRITE.size());
		m_history.push(TeHistory::PathPair(m_rootPath, QString()));
	}
	else {
		// No URI prefix: treat as an existing (read-only) archive.
		m_mode = MODE_READONLY;
		setArchive(path);
	}
}

QString TeArchiveFolderView::rootPath()
{
	return m_rootPath;
}

QString TeArchiveFolderView::archivePath() const
{
	return m_rootPath;
}

void TeArchiveFolderView::setCurrentPath(const QString & path)
{
	updatePath(path);
	m_history.push(TeHistory::PathPair(m_rootPath, path));
}

QString TeArchiveFolderView::currentPath()
{
	return indexToPath(mp_listProxy->mapToSource(mp_listView->rootIndex()));
}

void TeArchiveFolderView::moveNextPath()
{
	TeHistory::PathPair pair = m_history.next();
	if (pair != TeHistory::PathPair()) {
		updatePath(pair.second);
	}
}

void TeArchiveFolderView::movePrevPath()
{
	TeHistory::PathPair pair = m_history.previous();
	if (pair != TeHistory::PathPair()) {
		updatePath(pair.second);
	}
}

void TeArchiveFolderView::updatePath(const QString& path)
{
	QString cur = currentPath();
	if (currentPath() != path) {
		QString target = QDir::cleanPath(path);

		QStandardItem* entry = findPath(mp_treeModel->item(0), target);
		if (entry != nullptr) {
			mp_treeView->setCurrentIndex(mp_treeProxy->mapFromSource(mp_treeModel->indexFromItem(entry)));
			mp_listView->clearSelection();
			entry = findPath(mp_listModel->item(0), target);
			mp_listView->setRootIndex(mp_listProxy->mapFromSource(mp_listModel->indexFromItem(entry)));
		}
		else {
			mp_treeView->setCurrentIndex(mp_treeProxy->mapFromSource(mp_treeModel->index(0, 0)));
			mp_listView->setRootIndex(mp_listProxy->mapFromSource(mp_listModel->index(0, 0)));
		}
	}
}

QStringList TeArchiveFolderView::getPathHistory() const
{
	//usually, this list is used for Goto Folder function.
	//but in this class, it is used for history of archive folder and it is not compat for QDir.
	return QStringList();
}

TeFinder* TeArchiveFolderView::makeFinder()
{
	if(m_rootPath.isEmpty()) return nullptr;
	return new TeArchiveFinder(m_rootPath);
}

void TeArchiveFolderView::clear()
{
	if (mp_reader) {
		delete mp_reader;
		mp_reader = nullptr;
	}
	if (mp_tempDir) {
		delete mp_tempDir;
		mp_tempDir = nullptr;
	}

	m_stagedFiles.clear();
	m_stagedDirs.clear();

	mp_treeModel->clear();
	mp_listModel->clear();

	m_rootPath.clear();

	mp_treeModel->appendRow(createRootEntry());
	mp_listModel->appendRow(createRootEntry());

	mp_listView->setRootIndex(mp_listProxy->mapFromSource(mp_listModel->index(0,0)));

	//Hide headers of treeView.
	for (int i = 1; i < mp_treeView->header()->count(); i++) {
		mp_treeView->header()->setSectionHidden(i, true);
	}

	m_history.clear();
}

QString TeArchiveFolderView::tempPath()
{
	if (mp_tempDir == nullptr) {
		mp_tempDir = new QTemporaryDir();
	}
	if (!mp_tempDir->isValid()) {
		return QString();
	}
	return mp_tempDir->path();
}

QList<TeFileInfo> TeArchiveFolderView::entryInfo(const QStringList& virtualPaths)
{
	QList<TeFileInfo> result;
	QStandardItem* root = mp_listModel->item(0);
	if (root == nullptr) {
		return result;
	}
	for (const QString& path : virtualPaths) {
		QStandardItem* item = findPath(root, QDir::cleanPath(path));
		if (item != nullptr) {
			TeFileInfo info;
			info.importFromItem(item);
			result.append(info);
		}
	}
	return result;
}

void TeArchiveFolderView::rebuildStagingModel()
{
	QString cur = currentPath();

	mp_treeProxy->setDynamicSortFilter(false);
	mp_listProxy->setDynamicSortFilter(false);

	mp_treeModel->clear();
	mp_listModel->clear();
	mp_treeModel->appendRow(createRootEntry());
	mp_listModel->appendRow(createRootEntry());

	QString name = m_rootPath.isEmpty() ? tr("Archive") : QFileInfo(m_rootPath).fileName();
	mp_treeModel->item(0)->setData(name, Qt::DisplayRole);
	mp_listModel->item(0)->setData(name, Qt::DisplayRole);

	for (auto it = m_stagedDirs.constBegin(); it != m_stagedDirs.constEnd(); ++it) {
		internalAddDirEntry(*it);
	}
	for (auto it = m_stagedFiles.constBegin(); it != m_stagedFiles.constEnd(); ++it) {
		QFileInfo fi(it.value());
		internalAddEntry(it.key(), fi.size(), fi.lastModified(), 0x644);
	}

	mp_treeProxy->sort(0, mp_treeProxy->sortOrder());
	mp_listProxy->sort(0, mp_listProxy->sortOrder());
	mp_treeProxy->setDynamicSortFilter(true);
	mp_listProxy->setDynamicSortFilter(true);

	updatePath(cur);
}

bool TeArchiveFolderView::stageEntries(const QString& destDir, const QStringList& srcPaths)
{
	if (m_mode != MODE_WRITABLE) {
		return false;
	}

	QString base = destDir;
	if (!base.isEmpty() && !base.endsWith('/')) {
		base += '/';
	}

	bool added = false;
	for (const QString& src : srcPaths) {
		QFileInfo fi(src);
		if (!fi.exists()) {
			continue;
		}

		if (fi.isDir()) {
			QString dirRoot = base + fi.fileName();
			m_stagedDirs.insert(dirRoot);
			QDir srcDir(src);
			QDirIterator it(src,
				QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
				QDirIterator::Subdirectories);
			while (it.hasNext()) {
				QString child = it.next();
				QFileInfo ci(child);
				QString dest = dirRoot + "/" + srcDir.relativeFilePath(child);
				if (ci.isDir()) {
					m_stagedDirs.insert(dest);
				}
				else {
					m_stagedFiles.insert(dest, child);
				}
			}
			added = true;
		}
		else {
			m_stagedFiles.insert(base + fi.fileName(), src);
			added = true;
		}
	}

	if (added) {
		rebuildStagingModel();
	}
	return added;
}

bool TeArchiveFolderView::removeEntries(const QStringList& virtualPaths)
{
	if (m_mode != MODE_WRITABLE) {
		return false;
	}

	bool changed = false;
	for (const QString& vp : virtualPaths) {
		const QString sub = vp + "/";
		for (auto it = m_stagedFiles.begin(); it != m_stagedFiles.end(); ) {
			if (it.key() == vp || it.key().startsWith(sub)) {
				it = m_stagedFiles.erase(it);
				changed = true;
			}
			else {
				++it;
			}
		}
		for (auto it = m_stagedDirs.begin(); it != m_stagedDirs.end(); ) {
			if (*it == vp || it->startsWith(sub)) {
				it = m_stagedDirs.erase(it);
				changed = true;
			}
			else {
				++it;
			}
		}
	}

	if (changed) {
		rebuildStagingModel();
	}
	return changed;
}

bool TeArchiveFolderView::makeDirectory(const QString& destDir, const QString& name)
{
	if (m_mode != MODE_WRITABLE || name.isEmpty()) {
		return false;
	}

	QString base = destDir;
	if (!base.isEmpty() && !base.endsWith('/')) {
		base += '/';
	}
	m_stagedDirs.insert(base + name);
	rebuildStagingModel();
	return true;
}

bool TeArchiveFolderView::renameEntry(const QString& virtualPath, const QString& newName)
{
	if (m_mode != MODE_WRITABLE || newName.isEmpty()) {
		return false;
	}

	const int slash = virtualPath.lastIndexOf('/');
	const QString parent = (slash >= 0) ? virtualPath.left(slash + 1) : QString();
	const QString newPath = parent + newName;
	if (newPath == virtualPath) {
		return false;
	}

	const QString sub = virtualPath + "/";
	bool changed = false;

	QMap<QString, QString> updatedFiles;
	for (auto it = m_stagedFiles.constBegin(); it != m_stagedFiles.constEnd(); ++it) {
		if (it.key() == virtualPath) {
			updatedFiles.insert(newPath, it.value());
			changed = true;
		}
		else if (it.key().startsWith(sub)) {
			updatedFiles.insert(newPath + it.key().mid(virtualPath.size()), it.value());
			changed = true;
		}
		else {
			updatedFiles.insert(it.key(), it.value());
		}
	}
	m_stagedFiles = updatedFiles;

	QSet<QString> updatedDirs;
	for (const QString& d : m_stagedDirs) {
		if (d == virtualPath) {
			updatedDirs.insert(newPath);
			changed = true;
		}
		else if (d.startsWith(sub)) {
			updatedDirs.insert(newPath + d.mid(virtualPath.size()));
			changed = true;
		}
		else {
			updatedDirs.insert(d);
		}
	}
	m_stagedDirs = updatedDirs;

	if (changed) {
		rebuildStagingModel();
	}
	return changed;
}

bool TeArchiveFolderView::commit(const QString& destPath, TeArchive::ArchiveType type)
{
	if (m_mode != MODE_WRITABLE) {
		return false;
	}

	TeArchive::Writer writer;
	for (auto it = m_stagedFiles.constBegin(); it != m_stagedFiles.constEnd(); ++it) {
		if (!writer.addEntry(it.value(), it.key())) {
			return false;
		}
	}

	QProgressDialog progress(tr("Archiving..."), tr("Cancel"), 0, writer.count(), this);
	progress.setWindowTitle(tr("Archive"));
	progress.setWindowModality(Qt::WindowModal);
	QObject::connect(&writer, &TeArchive::Writer::maximumValue, &progress, &QProgressDialog::setMaximum);
	QObject::connect(&writer, &TeArchive::Writer::valueChanged, &progress, &QProgressDialog::setValue);
	QObject::connect(&writer, &TeArchive::Writer::currentFileInfoChanged, &progress, [&progress](const TeFileInfo& info) {
		progress.setLabelText(tr("Archive : ") + info.path.right(30));
	});
	QObject::connect(&writer, &TeArchive::Writer::finished, &progress, [&progress]() { progress.setValue(progress.maximum()); });
	QObject::connect(&progress, &QProgressDialog::canceled, &writer, &TeArchive::Writer::cancel);

	return writer.archive(destPath, type);
}

bool TeArchiveFolderView::setArchive(const QString & path)
{
	QDir dir;
	QString absPath = dir.absoluteFilePath(path);

	TeArchive::Reader* p_archive = new TeArchive::Reader();
	p_archive->open(absPath);

	// Handle encrypted archives: prompt for the password and retry until the
	// user supplies a correct one or cancels.
	bool needPassword = (p_archive->lastResult() == TeArchive::Reader::RESULT_PASSWORD_REQUIRED)
		|| (p_archive->lastResult() == TeArchive::Reader::RESULT_OK && p_archive->isEncrypted()
			&& p_archive->verifyPassword() != TeArchive::Reader::RESULT_OK);
	if (needPassword) {
		bool ok = false;
		while (true) {
			TePasswordDialog dialog(this);
			dialog.setPrompt(tr("Enter the password for \"%1\":").arg(QFileInfo(absPath).fileName()));
			if (dialog.exec() != QDialog::Accepted) {
				break; // cancelled by the user
			}
			p_archive->setPassword(dialog.password());
			if (p_archive->verifyPassword() == TeArchive::Reader::RESULT_OK) {
				ok = true;
				break;
			}
		}
		if (!ok) {
			delete p_archive;
			return false;
		}
	}
	else if (p_archive->lastResult() != TeArchive::Reader::RESULT_OK) {
		delete p_archive;
		return false;
	}

	if (!setArchive(p_archive)) {
		delete p_archive;
		return false;
	}
	return true;
}

bool TeArchiveFolderView::newArchive(const QString& path) {
	clear();
	if (path.isEmpty()) {
		return false;
	}
	QFileInfo info(path);
	if (info.exists()) {
		return false;
	}

    return false;
}

bool TeArchiveFolderView::setArchive(TeArchive::Reader * p_archive)
{
	clear();
	if (p_archive == nullptr || p_archive->path().isEmpty()) {
		return false;
	}

	// Take ownership of the reader for the lifetime of the mount.
	mp_reader = p_archive;
	m_mode = MODE_READONLY;

	m_rootPath = p_archive->path();
	m_history.push(TeHistory::PathPair(m_rootPath, QString()));

	QFileInfo info(m_rootPath);

	mp_treeModel->item(0)->setData(info.fileName(), Qt::DisplayRole);
	mp_listModel->item(0)->setData(info.fileName(), Qt::DisplayRole);

	// Suspend dynamic re-sorting while bulk-loading entries for performance.
	mp_treeProxy->setDynamicSortFilter(false);
	mp_listProxy->setDynamicSortFilter(false);

	// Enumerating archive entries can be slow; show a busy progress dialog so
	// the user can distinguish processing from a frozen UI.
	QProgressDialog progress(tr("Opening archive \"%1\"...").arg(info.fileName()),
		QString(), 0, 0, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(500);

	int count = 0;
	for (const auto& entry : *p_archive) {
		switch (entry.type) {
		case TeFileInfo::EN_DIR:
			internalAddDirEntry(entry.path);
			break;
		case TeFileInfo::EN_FILE:
			internalAddEntry(entry.path, entry.size, entry.lastModified, entry.permissions);
			break;
		default:
			break;
		}
		if ((++count % 64) == 0) {
			QApplication::processEvents();
		}
	}

	mp_treeProxy->sort(0, mp_treeProxy->sortOrder());
	mp_listProxy->sort(0, mp_listProxy->sortOrder());
	mp_treeProxy->setDynamicSortFilter(true);
	mp_listProxy->setDynamicSortFilter(true);

	// Place the cursor on the first entry for keyboard parity with the
	// file-system folder view.
	QModelIndex srcRoot = mp_listModel->index(0, 0);
	if (mp_listModel->hasChildren(srcRoot)) {
		mp_listView->setCurrentIndex(mp_listProxy->mapFromSource(mp_listModel->index(0, 0, srcRoot)));
	}

	return true;
}

void TeArchiveFolderView::internalAddEntry(const QString & path, qint64 size, const QDateTime & lastModified, int permission)
{
	if (path.startsWith('/')) return;

	QStringList paths = QDir::cleanPath(path).split('/');

	QFileIconProvider iconProvider;

	mkpath(iconProvider,mp_treeModel->item(0),paths,false);
	QStandardItem* list = mkpath(iconProvider,mp_listModel->item(0),paths,true);

	if (list != nullptr && findChild(list, paths.last()) == nullptr) {
		TeFileInfo info;
		info.type = TeFileInfo::EN_FILE;
		info.path = path;
		info.size = size;
		info.lastModified = lastModified;
		info.permissions = permission;
		list->appendRow(info.exportItem(false));
	}
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

	QStringList paths = destPath.split('/');

	QFileIconProvider iconProvider;
	mkpath(iconProvider, mp_treeModel->item(0), paths,false);
	mkpath(iconProvider, mp_listModel->item(0), paths,true);
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

	for (int i = 0; i < paths.size(); i++) {
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
	for (int i = 0; i < paths.size() - 1; i++) {
		QStandardItem* child = findChild(parent,paths[i]);
		if (child != nullptr) {
			parent = child;
		}
		else{
			QString path = "";
			for (int j = 0; j < i; j++) {
				path += paths[j] + "/";
			}
			QString parentPath = path.left(path.size() == 0 ? 0 : path.size() - 1);
			if (path.size() > 0){
				path = path + "/" + paths[i];
			}else{
				path = paths[i];
			}
			TeFileInfo info;
			info.type = TeFileInfo::EN_DIR;
			info.path = path;
			info.size = 0;
			info.lastModified = QDateTime();
			info.permissions = 0x755;

			QList<QStandardItem*> entries = info.exportItem(false);
			parent->appendRow(entries);
			parent = entries[0];
			if (bParentEntry == true) {
				parent->appendRow(createParentEntry(parentPath));
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
	entry->setData("", TeFileInfo::ROLE_PATH);
	entry->setData(tr("Archive"), TeFileInfo::ROLE_NAME);
	entry->setData(0x755, TeFileInfo::ROLE_PERM);
	entry->setData(TeFileInfo::EN_NONE, TeFileInfo::ROLE_TYPE);
	entry->setData(0, TeFileInfo::ROLE_SIZE);
	entry->setData(QDateTime(), TeFileInfo::ROLE_DATE);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem("");
	entries.append(entry);

	// Type
	entry = new TeStandardFileItem("");
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem("");
	entries.append(entry);

	return entries;
}

QList<QStandardItem*> TeArchiveFolderView::createParentEntry(const QString& path)
{
	QList<QStandardItem*> entries;

	// Entry Name and referenced path.
	QStandardItem* entry = new TeStandardFileItem("..");
	entry->setData(path, TeFileInfo::ROLE_PATH);
	entry->setData("..", TeFileInfo::ROLE_NAME);
	entry->setData(0x755, TeFileInfo::ROLE_PERM);
	entry->setData(TeFileInfo::EN_PARENT, TeFileInfo::ROLE_TYPE);
	entry->setData(0, TeFileInfo::ROLE_SIZE);
	entry->setData(QDateTime(), TeFileInfo::ROLE_DATE);
	entry->setEditable(false);
	entries.append(entry);

	// Size
	entry = new TeStandardFileItem("");
	entries.append(entry);

	// Type
	entry = new TeStandardFileItem("");
	entries.append(entry);

	// lastModified Date
	entry = new TeStandardFileItem("");
	entries.append(entry);

	return entries;
}

void TeArchiveFolderView::showContextMenu(const QAbstractItemView * pView, const QPoint & pos)
{
	if (pView == nullptr) return;

	// Archive entries are virtual; the OS shell menu cannot operate on them, so
	// always present the user-defined (TeMenuSetting) menu.  The commands
	// themselves decide what is applicable based on the current selection/mode.
	QPoint gpos = pView->mapToGlobal(pos);
	if (pView == mp_treeView) {
		showUserContextMenu(SETTING_TREEPOPUP_GROUP, gpos);
	}
	else if (pView == mp_listView) {
		showUserContextMenu(SETTING_LISTPOPUP_GROUP, gpos);
	}
}

void TeArchiveFolderView::showUserContextMenu(const QString& menuName, const QPoint& pos)
{
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
			QAction* action = new QAction(p_info->icon(), p_info->name());
			connect(action, &QAction::triggered, [this, cmdId](bool /*checked*/) { emit requestCommand(cmdId, TeTypes::WT_NONE, nullptr, nullptr); });
			menus.top()->addAction(action);
		}
	}
	settings.endGroup();
	settings.endGroup();

	menu.exec(pos);
}

void TeArchiveFolderView::itemActivated(const QModelIndex & index)
{
	QModelIndex srcIndex = mp_listProxy->mapToSource(index);
	int type = srcIndex.data(TeFileInfo::ROLE_TYPE).toInt();
	if (type == TeFileInfo::EN_DIR || type == TeFileInfo::EN_PARENT) {
		setCurrentPath(indexToPath(srcIndex));
	}
}

void TeArchiveFolderView::setFileShowMode(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed)
{
	// Hidden/system type flags do not apply to archive entries; only the sort
	// order is honoured here.
	Q_UNUSED(typeFlags);

	Qt::SortOrder sortOrder = orderReversed ? Qt::DescendingOrder : Qt::AscendingOrder;

	// The sort criterion is a TeTypes::OrderType; the proxy applies the
	// type-aware comparison (directory-first, name/size/date/extension) in
	// lessThan(), mirroring TeFileFolderView.
	mp_treeProxy->setSortType(order);
	mp_listProxy->setSortType(order);

	mp_treeProxy->sort(0, sortOrder);
	mp_listProxy->sort(0, sortOrder);
}