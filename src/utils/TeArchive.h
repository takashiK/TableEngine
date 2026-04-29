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

#include "TeFileInfo.h"

/**
 * @file TeArchive.h
 * @brief Archive read/write interface for supported archive formats.
 * @ingroup utility
 *
 * @details Declares TeArchive::Reader (extraction) and TeArchive::Writer
 * (creation) inside the TeArchive namespace.
 *
 * @see doc/markdown/utils/TeArchive.md
 */

class QFileInfo;
class QFile;

namespace TeArchive {

/** @brief Archive format type tag. */
enum ArchiveType {
	AR_NONE,        ///< Unknown file.
	AR_ZIP,         ///< ZIP archive.
	AR_7ZIP,        ///< 7-Zip archive.
	AR_TAR,         ///< Tar archive.
	AR_TAR_GZIP,    ///< Tar archive with gzip compression.
	AR_TAR_BZIP2,   ///< Tar archive with bzip2 compression.
};

/**
 * @class TeArchive::Reader
 * @brief Reads (extracts) entries from a supported archive file.
 * @ingroup utility
 *
 * @details Supports iteration over archive entries via a C++11-style
 * forward iterator (begin() / end()).  Extraction is performed by
 * extractAll() or extract() — both run asynchronously and emit
 * progress signals.
 *
 * @see TeArchive::Writer, doc/markdown/utils/TeArchive.md
 */
class Reader :
	public QObject
{
	Q_OBJECT

public:
	Reader();
	Reader(const QString& path);
	virtual ~Reader();

	/**
	 * @brief Installs an overwrite-confirmation callback.
	 *
	 * @p overwrite is called with the conflicting QFileInfo before overwriting.
	 * Return true to allow overwriting.
	 * @param overwrite Callback function pointer.
	 */
	void setCallback( bool(*overwrite)(QFileInfo*) );

	/**
	 * @brief Opens an archive file.
	 * @param path Absolute path to the archive.
	 * @return true on success.
	 */
	bool open( const QString& path);
	/** @brief Closes the archive and releases all resources. */
	void close();
	/** @brief Clears the internal cancellation flag before a new extraction. */
	void clearCancel();

	/** @brief Returns the path of the currently opened archive. */
	const QString& path() { return m_path; }
	/** @brief Returns true when an archive is open and valid. */
	bool isValid();
	/** @brief Returns the detected archive format. */
	ArchiveType type();

public slots:
	/**
	 * @brief Extracts all entries to @p destPath.
	 * @param destPath Absolute destination directory.
	 * @return true on success.
	 */
	bool extractAll(const QString& destPath);

	/**
	 * @brief Extracts a subset of entries.
	 * @param destPath Absolute destination directory.
	 * @param base     Common path prefix to strip from entry paths.
	 * @param entries  List of entry paths to extract.
	 * @return true on success.
	 */
	bool extract( const QString& destPath, const QString& base, const QStringList& entries);

	/** @brief Requests cancellation of an in-progress extraction. */
	void cancel();

signals:
	/**
	 * @brief Provides the total archive byte count for progress tracking.
	 *
	 * Emitted after extractAll() or extract() starts.  The value equals the
	 * archive file size (in KB), which is used as the progress maximum.
	 * @param value Archive file size in KB.
	 */
	void maximumValue(int value);

	/**
	 * @brief Provides a progress update during extraction.
	 *
	 * Indicates bytes read from the archive file.  Does not include
	 * decompression or file-write time, and will not reach maximumValue
	 * exactly (~10 KB remain).
	 * @param value Bytes read from the archive so far (in KB).
	 */
	void valueChanged(int value);

	/**
	 * @brief Emitted when extraction of a new entry begins.
	 * @param info Metadata of the entry being extracted.
	 */
	void currentFileInfoChanged(const TeFileInfo& info);

	/** @brief Emitted when extraction completes (success or cancellation). */
	void finished();

public:
	/** @brief Forward iterator over archive entries. */
	class const_iterator {
	public:
		const_iterator() : data(Q_NULLPTR) {}
		const_iterator(Reader* ar);
		const_iterator(const_iterator &&o) noexcept;
		~const_iterator();
		const TeFileInfo &operator*() const { return info; }
		const TeFileInfo *operator->() const { return &info; }
		bool operator==(const const_iterator &o) const Q_DECL_NOTHROW { return (data == o.data) && (info.path == o.info.path); }
		bool operator!=(const const_iterator &o) const Q_DECL_NOTHROW { return !(*this == o); }
		const_iterator &operator++();
	private:
		TeFileInfo info;
		void* data;
	};
	friend const_iterator;

	/** @brief Returns an iterator to the first archive entry. */
	const_iterator begin();
	/** @brief Returns the past-the-end iterator. */
	const_iterator end();

private:
	QString     m_path;             ///< Path of the opened archive.
	ArchiveType m_type;             ///< Detected archive format.
	bool        m_cancel;           ///< Cancellation flag.
	bool(*overwrite_check)(QFileInfo*); ///< Optional overwrite callback.

	bool copy_data(void* arPtr, QFile* ofile);
};

/**
 * @class TeArchive::Writer
 * @brief Creates archive files from a registered list of source entries.
 * @ingroup utility
 *
 * @details Files and directories are registered via addEntry() / addEntries().
 * The actual archive is written by archive(), which runs synchronously and
 * emits progress signals.
 *
 * @see TeArchive::Reader, doc/markdown/utils/TeArchive.md
 */
class Writer :
	public QObject
{
	Q_OBJECT

public:
	Writer();
	virtual ~Writer();

	void clear();
	int count();
	void clearCancel();

	bool addEntry(const QString& src, const QString& dest);
	bool addEntries(const QString& base, const QStringList& srcList, const QString& dest=QString());

public slots:
	bool archive(const QString& dest, ArchiveType type);
	void cancel();

signals:
	/** @brief Emitted for each entry registered by addEntry() or addEntries(). */
	void addedFileInfo(const TeFileInfo& info);

	/**
	 * @brief Provides the total source-file byte count for progress tracking.
	 *
	 * Emitted at the start of archive().
	 * @param value Total source bytes in KB.
	 */
	void maximumValue(int value);

	/**
	 * @brief Provides a progress update during archiving.
	 *
	 * Reports bytes read from source files.
	 * @param value Bytes read so far (in KB).
	 */
	void valueChanged(int value);

	/**
	 * @brief Emitted when archiving of a new entry begins.
	 * @param info Metadata of the entry being archived.
	 */
	void currentFileInfoChanged(const TeFileInfo& info);

	/** @brief Emitted when the archive operation completes. */
	void finished();

private:

	struct ArchiveInfo {
		TeFileInfo::EntryType type;
		QString src;
		QString dst;
		qint64 size;

		bool operator==(const ArchiveInfo& o) const {
			return (type == o.type) && (src == o.src) && (dst == o.dst);
		}
	};
	QList<ArchiveInfo> m_entryList;
	qint64 m_totalBytes;
	bool m_cancel;
};
}
