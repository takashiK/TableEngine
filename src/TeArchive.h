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
#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>

class QFileInfo;

namespace TeArchive {

//! Archive entry type descripter.
enum EntryType {
	EN_NONE,	//!< Undefined type
	EN_FILE,	//!< File type
	EN_DIR,		//!< Directory type
	EN_PARENT,	//!< Parent directory type
};

//! Archive entry descripter.
class FileInfo {
public:
	FileInfo() :type(EN_NONE), size(0) {}
	bool operator==(const FileInfo& info) const Q_DECL_NOTHROW { return path == info.path;  }
	bool operator!=(const FileInfo& info) const Q_DECL_NOTHROW  { return !(*this == info); }
	bool operator<(const FileInfo& info) const Q_DECL_NOTHROW { return path < info.path; }
	bool operator>(const FileInfo& info) const Q_DECL_NOTHROW { return info < *this; }
	bool operator<=(const FileInfo& info) const Q_DECL_NOTHROW { return !(*this > info); }
	bool operator>=(const FileInfo& info) const Q_DECL_NOTHROW { return !(*this < info); }

	bool operator==(const QString& rpath) const Q_DECL_NOTHROW { return path == rpath; }
	bool operator!=(const QString& rpath) const Q_DECL_NOTHROW { return path != rpath; }
	bool operator<(const QString& rpath) const Q_DECL_NOTHROW { return path < rpath; }
	bool operator>(const QString& rpath) const Q_DECL_NOTHROW { return path > rpath; }
	bool operator<=(const QString& rpath) const Q_DECL_NOTHROW { return path <= rpath; }
	bool operator>=(const QString& rpath) const Q_DECL_NOTHROW { return path >= rpath; }

	EntryType type;			//!< Entry type descripter.
	QString   path;			//!< Path in archive.
	QString   src;			//!< Source path of entry.
	qint64    size;			//!< File size.
	QDateTime lastModifyed;	//!< File last modification time.
};

//! Archive type descripter.
enum ArchiveType {
	AR_NONE,		//!< Unkown file.
	AR_ZIP,			//!< Zip file
	AR_7ZIP,		//!< 7 zip file
	AR_TAR,			//!< Tar archive.
	AR_TAR_GZIP,	//!< Tar archive with gzip compress.
	AR_TAR_BZIP2,	//!< Tar archive with bzip2 compress.
};

class Reader :
	public QObject
{
	Q_OBJECT

public:
	Reader();
	Reader(const QString& path);
	virtual ~Reader();

	void setCallback( bool(*overwrite)(QFileInfo*) );
	bool open( const QString& path);
	void close();

	const QString& path() { return m_path; }

	bool extractAll(const QString& destPath);
	bool extract( const QString& destPath, const QString& base, const QStringList& entries);

	ArchiveType type();

public:
	class const_iterator {
	public:
		const_iterator() : data(Q_NULLPTR) {}
		const_iterator(Reader* ar);
		const_iterator(const_iterator &&o);
		~const_iterator();
		const FileInfo &operator*() const { return info; }
		const FileInfo *operator->() const { return &info; }
		bool operator==(const const_iterator &o) const Q_DECL_NOTHROW { return (data == o.data) && (info.path == o.info.path); }
		bool operator!=(const const_iterator &o) const Q_DECL_NOTHROW { return !(*this == o); }
		const_iterator &operator++();
	private:
		FileInfo info;
		void* data;
	};
	friend const_iterator;

	const_iterator begin();
	const_iterator end();

private:
	QString m_path;
	ArchiveType m_type;
	bool(*overwrite_check)(QFileInfo*);
};

class Writer :
	public QObject
{
	Q_OBJECT

public:
	Writer();
	virtual ~Writer();

	void clear();
	int count();

	bool addEntry(const QString& src, const QString& dest);
	bool addEntries(const QString& base, const QStringList& srcList, const QString& dest=QString());

	bool archive(const QString& dest, ArchiveType type);

private:
	QList<FileInfo> m_entryList;
	bool m_sortFlag;
};
}
