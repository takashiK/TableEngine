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

#pragma once

#include "TeFolderView.h"
#include "utils/TeHistory.h"
#include "utils/TeFileInfo.h"

#include <QModelIndex>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QSet>

/**
 * @file TeArchiveFolderView.h
 * @brief Two-pane folder view backed by an in-memory archive model.
 * @ingroup widgets
 *
 * @see doc/markdown/widgets/TeArchiveFolderView.md
 */

class TeEventFilter;
class TeFileSortProxyModel;
class QAbstractItemView;
class QStandardItem;
class QStandardItemModel;
class QFileIconProvider;
class QPoint;
class QTemporaryDir;

namespace TeArchive {
	enum ArchiveType;
	class Reader;
	class Writer;
}

/**
 * @class TeArchiveFolderView
 * @brief TeFolderView implementation for navigating archive files.
 * @ingroup widgets
 *
 * @details Presents the contents of a single archive file as a virtual
 * directory tree.  An in-memory QStandardItemModel is built from the archive
 * entries; the same model is shared by both tree and list panes.
 *
 * The static constants URI_WRITE and URI_READ are URI-scheme prefixes used
 * to distinguish writable and read-only archive paths throughout the
 * application.
 *
 * @see TeFolderView, TeArchive::Reader, TeArchive::Writer
 * @see doc/markdown/widgets/TeArchiveFolderView.md
 */
class TeArchiveFolderView : public TeFolderView
{
	Q_OBJECT
public:
	static const QString URI_WRITE; ///< URI scheme prefix for a writable archive mount.
	static const QString URI_READ;  ///< URI scheme prefix for a read-only archive mount.

	/** @brief Archive mount mode. */
	enum Mode {
		MODE_READONLY, ///< Browsing an existing archive (read-only).
		MODE_WRITABLE, ///< Composing a new archive (staging, write enabled).
	};

	//ROLE & COLUMN rule is same as TeFileInfo

public:
	TeArchiveFolderView(QWidget *parent = Q_NULLPTR);
	~TeArchiveFolderView();

	/** @brief Returns TeTypes::WT_ARCHIVEVIEW. */
	virtual TeTypes::WidgetType getType() const;

	/** @brief Returns the tree-view pane. */
	virtual TeFileTreeView* tree();
	/** @brief Returns the list-view pane. */
	virtual TeFileListView* list();

	/**
	 * @brief Sets the URI of the archive currently mounted.
	 * @param path URI with URI_WRITE or URI_READ prefix followed by the
	 *             archive file path.
	 */
	virtual void setRootPath(const QString& path);
	/** @brief Returns the URI of the currently mounted archive. */
	virtual QString rootPath();

	/**
	 * @brief Navigates the list view to @p path within the archive.
	 * @param path Virtual path inside the archive (relative to archive root).
	 */
	virtual void setCurrentPath(const QString& path);
	/** @brief Returns the virtual path currently shown in the list view. */
	virtual QString currentPath();

	/** @brief Navigates forward in the history stack. */
	virtual void moveNextPath();
	/** @brief Navigates backward in the history stack. */
	virtual void movePrevPath();
	/** @brief Returns the virtual-path navigation history. */
	virtual QStringList getPathHistory() const;

	/** @brief Creates and returns a TeArchiveFinder for this view. */
	virtual TeFinder* makeFinder();

	/** @brief Clears the in-memory model and releases the archive handle. */
	void clear();

	/**
	 * @brief Mounts an archive from its file-system path.
	 * @param path Absolute path to the archive file.
	 * @return true on success.
	 */
	bool setArchive(const QString& path);

	/**
	 * @brief Creates a new writable archive at @p path and mounts it.
	 * @param path Absolute path to the new archive file.
	 * @return true on success.
	 */
	bool newArchive(const QString& path);

	/**
	 * @brief Mounts a pre-opened archive reader.
	 *
	 * Ownership of @p p_archive is transferred to this view.
	 * @param p_archive A fully opened TeArchive::Reader instance.
	 * @return true on success.
	 */
	bool setArchive(TeArchive::Reader* p_archive);
	/** @brief Returns the current mount mode. */
	Mode mode() const { return m_mode; }
	/** @brief Returns true when the mount is read-only (existing archive). */
	bool isReadOnly() const { return m_mode == MODE_READONLY; }

	/** @brief Returns the reader backing a read-only mount (may be null). */
	TeArchive::Reader* reader() const { return mp_reader; }

	/** @brief Returns the absolute archive file path (without URI prefix). */
	QString archivePath() const;

	/**
	 * @brief Returns the TeFileInfo describing each of the given virtual paths.
	 * @param virtualPaths Virtual paths inside the archive.
	 * @return TeFileInfo for every path found in the model (missing ones skipped).
	 */
	QList<TeFileInfo> entryInfo(const QStringList& virtualPaths);

	/**
	 * @brief Returns a temporary directory owned by this view.
	 *
	 * The directory is created lazily and removed when the view is destroyed
	 * or clear() is called.  Used for extracting entries during copy/paste.
	 * @return Absolute path of the temporary directory, or empty on failure.
	 */
	QString tempPath();

	// --- Writable-mode staging operations (MODE_WRITABLE only) ---------------

	/**
	 * @brief Stages filesystem files/directories into the archive at @p destDir.
	 * @param destDir   Virtual destination directory inside the archive.
	 * @param srcPaths  Absolute filesystem paths of files/directories to add.
	 * @return true if at least one entry was staged.
	 */
	bool stageEntries(const QString& destDir, const QStringList& srcPaths);

	/**
	 * @brief Removes staged entries (and their children) from the archive view.
	 * @param virtualPaths Virtual paths of entries to remove.
	 * @return true on success.
	 */
	bool removeEntries(const QStringList& virtualPaths);

	/**
	 * @brief Creates an empty staged directory at @p destDir/@p name.
	 * @return true on success.
	 */
	bool makeDirectory(const QString& destDir, const QString& name);

	/**
	 * @brief Renames a staged entry.
	 * @param virtualPath Current virtual path of the entry.
	 * @param newName     New leaf name.
	 * @return true on success.
	 */
	bool renameEntry(const QString& virtualPath, const QString& newName);

	/**
	 * @brief Commits all staged entries to an archive file.
	 * @param destPath Absolute output archive path.
	 * @param type     Archive format.
	 * @return true on success.
	 */
	bool commit(const QString& destPath, TeArchive::ArchiveType type);
public slots:
	/**
	 * @brief Updates file-type visibility flags and sort order.
	 * @param typeFlags     File type visibility flags.
	 * @param order         Sort column.
	 * @param orderReversed true for descending sort.
	 */
	virtual void setFileShowMode(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed);

protected:
	/**
	 * @brief Updates both panes to display directory @p path.
	 * @param path Virtual path inside the archive.
	 */
	void updatePath(const QString& path);

	/** @brief Inserts a file entry into the in-memory model. */
	void internalAddEntry(const QString& path, qint64 size, const QDateTime& lastModified, int permission);
	/** @brief Inserts a directory entry into the in-memory model. */
	void internalAddDirEntry(const QString& path);

	/** @brief Rebuilds the model from the staging maps (Writable mode). */
	void rebuildStagingModel();

	/** @brief Converts a list-view model index to its virtual archive path. */
	QString indexToPath(const QModelIndex &index);

	/** @brief Finds the model item corresponding to @p path. */
	QStandardItem* findPath(QStandardItem* root, const QString& path);

	/** @brief Creates intermediate directory items so that @p paths can be inserted. */
	QStandardItem* mkpath(const QFileIconProvider& iconProvider, QStandardItem* root, const QStringList& paths, bool bParentEntry);

	/** @brief Finds a direct child item by name under @p parent. */
	QStandardItem* findChild(const QStandardItem* parent, const QString& name);

	/** @brief Creates the root-level row items for the tree. */
	QList<QStandardItem*> createRootEntry();

	/** @brief Creates the ".." parent-directory row items for @p path. */
	QList<QStandardItem*> createParentEntry(const QString& path);

	/** @brief Shows a custom (TeMenuSetting-based) context menu at @p pos. */
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos);

	/** @brief Builds and shows a user-defined context menu from settings. */
	void showUserContextMenu(const QString& menuName, const QPoint& pos);

protected slots:
	/**
	 * @brief Navigates the list view when a tree or list item is activated.
	 * @param index The activated model index.
	 */
	void itemActivated(const QModelIndex &index);

private:
	QString m_rootPath;              ///< URI of the currently mounted archive.
	Mode m_mode = MODE_WRITABLE;     ///< Current mount mode.

	TeArchive::Reader* mp_reader = nullptr; ///< Reader for a read-only mount (owned).
	QTemporaryDir* mp_tempDir = nullptr;    ///< Temp dir for extraction (owned, lazy).

	QMap<QString, QString> m_stagedFiles;   ///< Writable staging: dest virtual path -> source FS path.
	QSet<QString> m_stagedDirs;             ///< Writable staging: empty directory virtual paths.

	QStandardItemModel* mp_treeModel;   ///< Source model backing the tree pane.
	QStandardItemModel* mp_listModel;   ///< Source model backing the list pane.
	TeFileSortProxyModel* mp_treeProxy; ///< Sort/filter proxy between source model and tree pane.
	TeFileSortProxyModel* mp_listProxy; ///< Sort/filter proxy between source model and list pane.

	TeFileTreeView* mp_treeView;     ///< Left-pane tree widget.
	TeFileListView* mp_listView;     ///< Right-pane list widget.

	TeEventFilter* mp_treeEvent;     ///< Event filter for the tree pane.
	TeEventFilter* mp_listEvent;     ///< Event filter for the list pane.

	TeHistory m_history;             ///< Virtual-path navigation history.
};
