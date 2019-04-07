/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include "TeFolderView.h"
#include <QModelIndex>

class TeEventFilter;
class QAbstractItemView;
class QStandardItem;
class QFileIconProvider;
class QPoint;

namespace TeArchive {
	enum ArchiveType;
	class Reader;
	class Writer;
}

class TeArchiveFolderView : public TeFolderView
{
	Q_OBJECT
public:
	static const QString URI_WRITE;
	static const QString URI_READ;

	enum EntryColmun {
		COL_NAME,
		COL_SIZE,
		COL_TYPE,
		COL_DATE,
	};

public:
	TeArchiveFolderView(QWidget *parent = Q_NULLPTR);
	~TeArchiveFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

	void clear();
	bool setArchive(const QString& path);
	bool setArchive(TeArchive::Reader* p_archive);

	TeArchive::Writer* archive();
	bool archive(const QString& path, TeArchive::ArchiveType type);

	void addEntry(const QString& path, qint64 size, const QDateTime& lastModified, const QString& src);
	void addDirEntry(const QString& path);

protected:
	void internalAddEntry(const QString& path, qint64 size, const QDateTime& lastModified, const QString& src);
	void internalAddDirEntry(const QString& path);
	QString indexToPath(const QModelIndex &index);
	QStandardItem* findPath(QStandardItem* root, const QString& path);
	QStandardItem* mkpath(const QFileIconProvider& iconProvider, QStandardItem* root, const QVector<QStringRef>& paths, bool bParentEntry);
	QStandardItem* findChild(const QStandardItem* parent, const QString& name);
	QStandardItem* findChild(const QStandardItem* parent, const QStringRef& name);
	QList<QStandardItem*> createRootEntry();
	QList<QStandardItem*> createParentEntry();
	QList<QStandardItem*> createDirEntry(const QFileIconProvider& iconProvider, const QString& name);
	QList<QStandardItem*> createFileEntry(const QFileIconProvider& iconProvider, const QString& name, qint64 size, const QDateTime& lastModified, const QString& src);

	void buildArchiveEntry(TeArchive::Writer* writer, QStandardItem* rootItem);

	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos) const;

protected slots:
	void itemActivated(const QModelIndex &index);

private:
	enum Mode {
		MODE_WRITE,
		MODE_READ,
	};
	
	Mode m_mode;
	QString m_rootPath;

	TeFileTreeView* mp_treeView;
	TeFileListView* mp_listView;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;
};
