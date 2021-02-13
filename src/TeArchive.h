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
#include <QObject>
#include <QString>
#include <QDateTime>

class QFileInfo;
class QFile;

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

	void cancel();
	void clearCancel();

	/*!
		Return target archive filepath.
	*/
	const QString& path() { return m_path; }
	ArchiveType type();

public slots:
	bool extractAll(const QString& destPath);
	bool extract( const QString& destPath, const QString& base, const QStringList& entries);

signals:
	/*!
		Provide a maximum byte counts for extraction from archive.
		This signal emit after call extractAll() or extract().
		This value is allways same as archive file size.
		so it return archive file size when you call extract() with partial entries.

		\param value archive file size  (unit by 1KB).
	*/
	void maximumValue(int value);
	/*!
		Provide progress value at extraction from archive.
		This value indidate read bytes from archive file by extraction function.
		this values has limitation below.
		\li It's not include decompression and write extracted data to file. 
		\li It's not reach to maximubValue. Remaining value is approximately 10KB.

		\param value readed bytes  (unit by 1KB).
	*/
	void valueChanged(int value);

	/*!
		Provide current extraction file information.
	*/
	void currentFileInfoChanged(const FileInfo& info);
	
	/*!
		extraction process is finished;
	*/
	void finished();

public:
	class const_iterator {
	public:
		const_iterator() : data(Q_NULLPTR) {}
		const_iterator(Reader* ar);
		const_iterator(const_iterator &&o) noexcept;
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
	bool m_cancel;
	bool(*overwrite_check)(QFileInfo*);

	bool copy_data(void* arPtr, QFile* ofile);
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
	void cancel();
	void clearCancel();

	bool addEntry(const QString& src, const QString& dest);
	bool addEntries(const QString& base, const QStringList& srcList, const QString& dest=QString());

public slots:
	bool archive(const QString& dest, ArchiveType type);

signals:
	/*!
		Provide a summentional byte counts of source files.
		This signal emit after call archive().

		\param summentional byte counts of source files (unit by 1KB).
	*/
	void maximumValue(int value);
	/*!
		Provide progress value for archiving.
		This value indidate read bytes from srouce files by archive().
		It's not include compression and write data to archive file.
		so if you use this value for progress then it reach 100% before totally complition for archive.
		but its difference is small so convenient for progress.

		\param value readed bytes  (unit by 1KB).
	*/
	void valueChanged(int value);

	/*!
		Provide current archving file information.
	*/
	void currentFileInfoChanged(const FileInfo& info);

	/*!
		archive process is finished.
	*/
	void finished();

private:
	QList<FileInfo> m_entryList;
	qint64 m_totalBytes;
	bool m_cancel;
};
}
