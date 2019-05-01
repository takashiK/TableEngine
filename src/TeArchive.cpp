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
#include "TeArchive.h"
#include "archive.h"
#include "archive_entry.h"

#include <QFile>
#include <QDir>
#include <algorithm>

/*!
	\namespace TeArchive
	\ingroup utility
	\brief This namespace provide operation classes for archive.

	This namespace provide operation classes for archive.
	These classes are depend on libarchive and provide it's functionarity.

	\li Example for extract from archive.
		\code
			TeArchive::Reader reader;
			reader.open("archive.zip");
			reader.extractAll("destination_path");
			reader.close();
		\endcode
	\li Example for create archive.
		\code
			TeArchive::Writer writer;
			writer.addEntry("target","path_in_archive");
			writer.archive("archive.zip",TeArchive::AR_ZIP);
		\endcode

 */

namespace {

	/*!
		\internal 

		Resource data for utility functions for libarchive.
		this class is sent to utility functions through client_data parameter.
		this class is setup by helper funtions open_read_archive() and open_write_archive().
		don't touch member variable directory.
	*/
	class QtArchiveResource
	{
	public:
		QtArchiveResource() { buffer = new char[buffer_size]; }
		~QtArchiveResource() { delete[] buffer; }
		QFile file;
		static const int buffer_size = 1024 * 128;
		char* buffer;
	};

	/*!
		\internal 

		This class is information object for libarchive.
		helper functions access to libarchive object through this class.
		don't touch member variable directory.
	 */
	class QtArchiveInfo
	{
	public:
		QtArchiveInfo() : ar(Q_NULLPTR), entry(Q_NULLPTR){}
		struct archive *ar;
		struct archive_entry *entry;
		QtArchiveResource res;
	};

	/*!
		\internal 

		Provide operation for "open file for read" to libarchive.

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
	 */
	extern "C" int qt_read_open_callback(struct archive *a, void *client_data)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		int result = ARCHIVE_OK;

		if (!qas->file.exists()) {
			result = ARCHIVE_FATAL;
			archive_set_error(a, EINVAL, "File not found.");
		}
		else if (!qas->file.open(QFile::ReadOnly)) {
			result = ARCHIVE_FATAL;
			archive_set_error(a, EINVAL, qas->file.errorString().toUtf8());
		}
		return result;
	}

	/*!
		\internal

		Provide operation for "close file" to libarchive.

		\param a not using here.
		\param client_data it is expected "Pointer of QtArchvieResource".
	*/
	extern "C" int qt_close_callback(struct archive* /*a*/, void *client_data)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);

		qas->file.close();
		return ARCHIVE_OK;
	}

	/*!
		\internal

		Provide operation for "sequencial read data of file" to libarchive.

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
		\param buffer memory buffer is provided by this function. this memory buffer addres is same as QtArchiveResource::buffer. it is provided from client_data.
	 */
	extern "C" la_ssize_t qt_read_callback(struct archive *a, void *client_data, const void **buffer)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		la_ssize_t result = 0;
		*buffer = nullptr;

		if (!qas->file.isOpen()) {
			result = -1;
			archive_set_error(a, EPERM, "File Not opened.");
		}
		else if (!qas->file.isReadable()) {
			result = -1;
			archive_set_error(a, EPERM, "File Not readable.");
		}
		else {
			result = qas->file.read(qas->buffer, qas->buffer_size);
			if (result < 0) {
				archive_set_error(a, EPERM, qas->file.errorString().toUtf8());
			}
			else {
				*buffer = qas->buffer;
			}
		}

		return result;
	}

	/*!
		\internal

		Provide operation for "skip data of file" to libarchive.
		Skip function is only provide positive skip. If you want to seek backward then you can use qt_seek_callback().

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
		\param request skip byte count. it permit only positive count.
	 */
	extern "C" la_int64_t qt_skip_callback(struct archive *a, void *client_data, la_int64_t request)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		la_int64_t result = 0;

		if (request < 0) {
			result = -1;
			archive_set_error(a, EPERM, "Can't skip with nagative offset.");
		}
		else {
			result = qas->file.skip(request);
			if (result < 0) {
				archive_set_error(a, EPERM, qas->file.errorString().toUtf8());
			}
		}

		return result;
	}

	/*!
		\internal

		Provide operation for "seek position of file" to libarchive.

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
		\param offset seek byte from whence point. it accept negative bytes.
		\param whence this param 3 variation below.
				\li \c SEEK_SET : Seek from top of file.
				\li \c SEEK_CUR : Seek from current position.
				\li \c SEEK_END : Seek from end of file.
	 */
	extern "C" la_int64_t qt_seek_callback(struct archive *a, void *client_data, la_int64_t offset, int whence)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		la_int64_t result = 0;

		qint64 size = qas->file.size();
		qint64 pos = qas->file.pos();
		qint64 target = 0;

		switch (whence) {
		case SEEK_SET:
			target = offset;
			break;
		case SEEK_CUR:
			target = pos + offset;
			break;
		case SEEK_END:
			target = size + offset;
			break;
		}

		if (target < 0) {
			target = 0;
		}
		else if (target > size) {
			target = size;
		}

		if (!qas->file.seek(target)) {
			archive_set_error(a, EPERM, "Can't seek file.");
			result = -1;
		}
		else {
			result = qas->file.pos();
		}

		return result;
	}

	/*!
		\internal

		Provide operation for "open file for write" to libarchive.

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
	 */
	extern "C" int qt_write_open_callback(struct archive *a, void *client_data)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		int result = ARCHIVE_OK;

		if (!qas->file.open(QFile::WriteOnly | QFile::Truncate)) {
			result = ARCHIVE_FATAL;
			archive_set_error(a, EINVAL, qas->file.errorString().toUtf8());
		}
		return result;
	}

	/*!
		\internal

		Provide operation for "write data to file" to libarchive.

		\param a state structure of libarchive.
		\param client_data it is expected "Pointer of QtArchvieResource".
		\param buffer data is written to archive by this function.
		\param length data length that in buffer.
	 */
	extern "C"	la_ssize_t	qt_write_callback(struct archive *a, void *client_data, const void *buffer, size_t length)
	{
		QtArchiveResource * qas = static_cast<QtArchiveResource*>(client_data);
		la_ssize_t result = 0;

		if (!qas->file.isOpen()) {
			result = -1;
			archive_set_error(a, EPERM, "File Not opened.");
		}
		else if (!qas->file.isWritable()) {
			result = -1;
			archive_set_error(a, EPERM, "File Not writable.");
		}
		else {
			result = qas->file.write(static_cast<const char*>(buffer), length);
			if (result < 0) {
				archive_set_error(a, EPERM, qas->file.errorString().toUtf8());
			}
		}

		return result;
	}

	/*!
		\internal

		helper function for "open archive for extract data from archive" by libarchive.

		\param arInfo this function setup this param to "Read from archive file" by \a path information.
		\param path   target for read archive file.
	 */
	bool open_read_archive(QtArchiveInfo* arInfo, const QString& path)
	{
		arInfo->ar = archive_read_new();
		archive_read_support_format_all(arInfo->ar);
		archive_read_support_compression_all(arInfo->ar);

		archive_read_set_seek_callback(arInfo->ar, qt_seek_callback);

		arInfo->res.file.setFileName(path.toLocal8Bit());

		int r = archive_read_open2(arInfo->ar, &arInfo->res, qt_read_open_callback, qt_read_callback, qt_skip_callback, qt_close_callback);
		if (r != ARCHIVE_OK) {
			archive_read_close(arInfo->ar);
			archive_read_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
			return false;
		}

		return true;
	}

	/*!
		\internal

		helper function for "close archive".
		if you open archive by open_read_archive() then you need call this function before destruct QtArchiveInfo. 

		\param arInfo read opened information structure.

	*/
	void close_read_archive(QtArchiveInfo* arInfo)
	{
		arInfo->entry = Q_NULLPTR;
		if (arInfo->ar != Q_NULLPTR) {
			archive_read_close(arInfo->ar);
			archive_read_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
		}
	}

	/*!
		\internal

		helper function for "check archive format".
		this function need call after archive_read_next_header().
		because this function use result of archive_read_next_header().
		helper function read_next_entry() is same effect as archive_read_next_header().

		\param arInfo read opened information structure.
		*/
	TeArchive::ArchiveType check_format(QtArchiveInfo* arInfo) {
		TeArchive::ArchiveType type = TeArchive::AR_NONE;
		switch (archive_format(arInfo->ar) & ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FORMAT_TAR:
			switch (archive_compression(arInfo->ar)) {
			case ARCHIVE_COMPRESSION_NONE:
				type = TeArchive::AR_TAR;
				break;
			case ARCHIVE_COMPRESSION_GZIP:
				type = TeArchive::AR_TAR_GZIP;
				break;
			case ARCHIVE_COMPRESSION_BZIP2:
				type = TeArchive::AR_TAR_BZIP2;
				break;
			default:
				break;
			}
			break;
		case ARCHIVE_FORMAT_ZIP:
			type = TeArchive::AR_ZIP;
			break;
		case ARCHIVE_FORMAT_7ZIP:
			type = TeArchive::AR_7ZIP;
			break;
		case ARCHIVE_FORMAT_CPIO:
		case ARCHIVE_FORMAT_SHAR:
		case ARCHIVE_FORMAT_ISO9660:
		case ARCHIVE_FORMAT_AR:
		case ARCHIVE_FORMAT_MTREE:
		case ARCHIVE_FORMAT_RAW:
		case ARCHIVE_FORMAT_XAR:
		case ARCHIVE_FORMAT_LHA:
		case ARCHIVE_FORMAT_CAB:
		case ARCHIVE_FORMAT_RAR:
		case ARCHIVE_FORMAT_WARC:
		default:
			break;
		}
		return type;
	}

	/*!
		\internal

		helper function for "read next FileInfo that inner data of archive".
		you can call this function afer open_read_archive().

		\param arInfo read opened information structure.
		\param info file informaition of next archive entry.
	 */
	bool read_next_entry(QtArchiveInfo* arInfo, TeArchive::FileInfo* info) {
		info->type = TeArchive::EN_NONE;
		info->size = 0;
		info->path.clear();
		info->lastModifyed = QDateTime::currentDateTime();

		while (ARCHIVE_OK == archive_read_next_header(arInfo->ar, &arInfo->entry)) {

			__LA_MODE_T mode = archive_entry_filetype(arInfo->entry);
			switch (mode) {
			case AE_IFREG:
				info->type = TeArchive::EN_FILE;
				break;
			case AE_IFDIR:
				info->type = TeArchive::EN_DIR;
				break;
			default:
				//reject other type.
				continue;
			}

			const char* p_path = archive_entry_pathname(arInfo->entry);
			const char* p_upath = archive_entry_pathname_utf8(arInfo->entry);
			if (p_upath != nullptr && p_upath[0] != 0) {
				info->path = QDir::cleanPath(QString::fromUtf8(p_upath));
			}
			else {
				info->path = QDir::cleanPath(QString::fromLocal8Bit(p_path));
			}

			if (info->path.startsWith(".") || info->path.startsWith("/") || info->path.contains(":")) {
				//reject unacceptable path
				info->type = TeArchive::EN_NONE;
				info->path.clear();
				continue;
			}

			if (archive_entry_size_is_set(arInfo->entry)) {
				info->size = archive_entry_size(arInfo->entry);
			}

			if (archive_entry_mtime_is_set(arInfo->entry)) {
				time_t mtime = archive_entry_mtime(arInfo->entry);
				info->lastModifyed.setTime_t(mtime);
			}
			return true;
		}
		return false;
	}

	/*!
		Return read bytes form archive by extraction.

		\return read bytes by 1 Kbyte (1024 byte) unit.
	*/
	inline int archive_read_bytes(QtArchiveInfo* arInfo)
	{
		la_int64_t read_bytes = archive_position_compressed(arInfo->ar);
		return read_bytes > 0 ? static_cast<int>( (read_bytes-1) / 1024 + 1) : 0;
	}
		
	/*!
		\internal

		helper function for "open archive file for create archive data" by libarchive

		\param arInfo this function setup this param to "Create archive file" by \a path information.
		\param path   path for creation.

	 */
	bool open_write_archvie(QtArchiveInfo* arInfo, const QString& path, TeArchive::ArchiveType type)
	{
		arInfo->ar = archive_write_new();

		switch(type){
		case TeArchive::AR_ZIP:
			archive_write_set_format_zip(arInfo->ar);
			break;
		case TeArchive::AR_7ZIP:
			archive_write_set_format_7zip(arInfo->ar);
			archive_write_set_format_option(arInfo->ar, "7zip", "compression", "lzma2");
			break;
		case TeArchive::AR_TAR:
			archive_write_set_format_gnutar(arInfo->ar);
			break;
		case TeArchive::AR_TAR_GZIP:
			archive_write_set_format_gnutar(arInfo->ar);
			archive_write_set_compression_gzip(arInfo->ar);
			break;
		case TeArchive::AR_TAR_BZIP2:
			archive_write_set_format_gnutar(arInfo->ar);
			archive_write_set_compression_bzip2(arInfo->ar);
			break;
		}

		arInfo->res.file.setFileName(path.toLocal8Bit());

		int r = archive_write_open(arInfo->ar, &arInfo->res, qt_write_open_callback, qt_write_callback, qt_close_callback);
		if (r != ARCHIVE_OK) {
			archive_write_close(arInfo->ar);
			archive_write_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
			return false;
		}

		return true;
	}

	/*!
		\internal

		helper function for "close archive".
		if you open archive by open_write_archive() then you need call this function before destruct QtArchiveInfo.

		\param arInfo write opened information structure.
	 */
	void close_write_archive(QtArchiveInfo* arInfo)
	{
		arInfo->entry = Q_NULLPTR;
		if (arInfo->ar != Q_NULLPTR) {
			archive_write_close(arInfo->ar);
			archive_write_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
		}
	}
}

namespace TeArchive {

/*!
	\class TeArchive::Reader
	\ingroup utility
	\brief Helper class for extract data from archvie.
	
	This is helper class for extract data from archive used by libarchive.
	This class currentry support .zip .7z .tar .tar.gz .tar.bz files.
	And and also potentialy support to formats that is supported by libarchive.

	This class auto detect format type of archvie. so you don't need set format type to this class.

	A simple usage is below.
	\code
		TeArchive::Reader reader;
		reader.open("archvie_file");
		reader.extractAll("destination_path");
		reader.close();
	\endcode
 */


Reader::Reader()
{
	overwrite_check = Q_NULLPTR;
	m_type = AR_NONE;
}

/*!
	Constructor of archive reader.
	it open \a path after construct.
 */
Reader::Reader(const QString & path)
{
	overwrite_check = Q_NULLPTR;
	m_type = AR_NONE;
	open(path);
}


Reader::~Reader()
{
	close();
}

/*!
	Set callback function that use for confirm "overwrite" when extracted file is already exist.

	Parameter of callback function is QFileInfo of extraction file. If callback function return true
	then this file is overwrite by extracted data. If callback function return false then
	Reader skip exctract relative data.
	If you want to change extract path. You can use QFileInfo::setFile() with return true.
 */
void Reader::setCallback( bool(*overwrite)(QFileInfo*) )
{
	overwrite_check = overwrite;
}

/*!
	Open archive file.
	If archive file is not found or Reader can't detect format of archive then this function failed and return false.
	This function close file, that already opened by this function, before open another file.
 */
bool Reader::open(const QString & path)
{
	bool result = false;
	close();
	if (!QFile::exists(path)) {
		return false;
	}

	QtArchiveInfo arInfo;

	if (!open_read_archive(&arInfo, path)) {
		return false;
	}

	if (ARCHIVE_OK == archive_read_next_header(arInfo.ar, &arInfo.entry)) {
		m_type = check_format(&arInfo);
		if (m_type != AR_NONE) {
			result = true;
			m_path = path;
		}
	}

	close_read_archive(&arInfo);

	return result;
}

/*!
	close archive file if it is already opened.
 */
void Reader::close()
{
	m_type = AR_NONE;
	m_path.clear();
}

/*!
	Extact all of data in archive to \a destPath.
	If any file success to extract then this function return true.
	but If all of data failed to extract then this function return false.

	\param destPath destination path.
 */
bool Reader::extractAll(const QString & destPath)
{
	FileInfo info;

	if (m_path.isEmpty()) {
		emit finished();
		return false;
	}
	
	QtArchiveInfo arInfo;

	QFileInfo srcInfo(m_path);
	if (!srcInfo.isReadable()) {
		emit finished();
		return false;
	}

	emit maximumValue(static_cast<int>( (srcInfo.size()-1)/1024+1));

	if (!open_read_archive(&arInfo, m_path)) {
		emit finished();
		return false;
	}

	QString destBase;
	if (destPath.endsWith('/')) {
		destBase = destPath;
	}
	else {
		destBase = destPath + "/";
	}

	QDir dir;
	while (read_next_entry(&arInfo, &info)) {

		emit currentFileInfoChanged(info);
		emit valueChanged(archive_read_bytes(&arInfo));

		if (info.type == EN_DIR) {
			dir.mkpath(destBase + info.path);
		}
		else if (info.type == EN_FILE) {
			QFileInfo fileInfo(destBase + info.path);
			if (fileInfo.exists()) {
				if ((overwrite_check != Q_NULLPTR) && !overwrite_check(&fileInfo)) {
					continue;
				}
			}

			if (!dir.exists(fileInfo.path())) {
				dir.mkpath(fileInfo.path());
			}

			QFile ofile(fileInfo.filePath());
			if (!ofile.open(QFile::WriteOnly | QFile::Truncate)) {
				continue;
			}

			copy_data(&arInfo, &ofile);

			ofile.close();
		}
	}

	close_read_archive(&arInfo);

	emit finished();
	return true;
}

/*!
	Extract selected data of archive to \a destPath.
	\a base is target path in archive. \a entries is list of file or directories path in archive
	that is relative from \a base.
	If you set directory in entries then thats children are also selected. 

	\param destPath destination path.
	\param base     base path in archive. all of \a entries are relative to this path.
	\param entries  extraction files list.
 */
bool Reader::extract(const QString & destPath, const QString & base, const QStringList & entries)
{
	FileInfo info;

	if (m_path.isEmpty()) {
		emit finished();
		return false;
	}

	QtArchiveInfo arInfo;

	if (!open_read_archive(&arInfo, m_path)) {
		emit finished();
		return false;
	}

	QString destBase;
	if (destPath.endsWith('/')) {
		destBase = destPath;
	}
	else {
		destBase = destPath + "/";
	}

	QDir dir;
	while (read_next_entry(&arInfo, &info)) {
		if (info.type == EN_DIR || info.type == EN_FILE) {
			if (!info.path.startsWith(base)) {
				continue;
			}
			bool found = false;
			for (auto&& entry : entries) {
				if (info.path.midRef(base.size()).startsWith(entry)) {
					found = true; break;
				}
			}
			if (!found) continue;
		}

		if (info.type == EN_DIR) {
			dir.mkpath(destBase + info.path);
		}
		else if (info.type == EN_FILE) {
			QFileInfo fileInfo(destBase + info.path);
			if (fileInfo.exists() && (overwrite_check != Q_NULLPTR) && !overwrite_check(&fileInfo)) {
				continue;
			}

			if (!dir.exists(fileInfo.path())) {
				dir.mkpath(fileInfo.path());
			}

			QFile ofile(fileInfo.filePath());
			if (!ofile.open(QFile::WriteOnly | QFile::Truncate)) {
				continue;
			}

			copy_data(&arInfo, &ofile);

			ofile.close();
		}
	}

	close_read_archive(&arInfo);

	emit finished();
	return true;
}

/*!
	helper function for "copy data that in archive to file".
	this function copy current entry data to file. so before call this function.
	you need set target entry by read_next_entry().

	\param arPtr  Pointer of QtArchiveInfo. QtArchiveInfo is not defined outside of this file, so we use void pointer.
	\param ofile  Extract current archvie entry to this file.
 */
bool Reader::copy_data(void* arPtr, QFile* ofile)
{
	int r;
	const void* buff;
	size_t size = 0;
	la_int64_t offset = 0;

	QtArchiveInfo* arInfo = static_cast<QtArchiveInfo*>(arPtr);

	for (;;) {
		r = archive_read_data_block(arInfo->ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return true;
		if (r != ARCHIVE_OK)
			return false;

		qint64 wsize = ofile->write(static_cast<const char*>(buff), size);
		if (wsize < 0 || wsize < static_cast<qint64>(size)) {
			return false;
		}
		
		emit valueChanged(archive_read_bytes(arInfo));
	}
}

/*!
	Return the type of archive format.
 */
ArchiveType Reader::type()
{
	return m_type;
}

/*!
	Return the iterator. It use for check entry in archive and point to begin of archvie entry.
 */
Reader::const_iterator Reader::begin()
{
	return const_iterator(this);
}

/*!
	Return the iterator. It point to end of archive entry.
 */
Reader::const_iterator Reader::end()
{
	return const_iterator();
}

/*!
	constructor of iterator that use for check entry in archive.
	\a ar is target class for information.
 */
Reader::const_iterator::const_iterator(Reader * ar)
{
	data = nullptr;
	if (!ar->m_path.isEmpty()) {
		QtArchiveInfo* arInfo = new QtArchiveInfo;
		data = arInfo;

		open_read_archive(arInfo, ar->m_path);
		if (!read_next_entry(arInfo, &info)) {
			close_read_archive(arInfo);
			delete arInfo;
			data = Q_NULLPTR;
		}
	}
}

/*!
	copy constructor of iterator that use for check entry in archive.
	it copy from \a o.
 */
Reader::const_iterator::const_iterator(const_iterator && o)
{
	data = o.data;
	info = o.info;

	o.data = Q_NULLPTR;
	o.info.type = EN_NONE;
	o.info.size = 0;
}

Reader::const_iterator::~const_iterator()
{
	if (data != Q_NULLPTR) {
		QtArchiveInfo* arInfo = static_cast<QtArchiveInfo*>(data);
		close_read_archive(arInfo);
		delete arInfo;
		
		data = Q_NULLPTR;
		info.type = EN_NONE;
		info.size = 0;
	}
}

Reader::const_iterator& Reader::const_iterator::operator++()
{
	if (data != Q_NULLPTR) {
		QtArchiveInfo* arInfo = static_cast<QtArchiveInfo*>(data);
		if (!read_next_entry(arInfo, &info)) {
			close_read_archive(arInfo);
			delete arInfo;
			data = Q_NULLPTR;
		}
	}
	return *this;
}

/*!
	\class TeArchive::Writer
	\ingroup utility
	\brief Helper class for create archvie from files.

	This is helper class for create archive from files used by libarchive.
	This class currentry support .zip .7z .tar .tar.gz .tar.bz files.
	And and also potentialy support to formats that is supported by libarchive.

	A simple usage for zip file is below.
	\code
		TeArchive::Writer writer;
		writer.addEntry("path_of_files","");
		writer.archive("name_of_archive_file",TeArchvie::AR_ZIP);
	\endcode
 */
Writer::Writer()
{
	m_sortFlag = true;
}


Writer::~Writer()
{
}

/*!
	Clear entries.
 */
void Writer::clear()
{
	m_entryList.clear();
}

/*!
	Count of entries. usually this function return different count to number of call addEntry() and count of entries in addEntries().
	Because this count is include children of directory.
 */
int Writer::count()
{
	return m_entryList.size();
}

/*!
	Add file or directory to \a dest path in archive.

	\param src  target file. it include archive file after call archive().
	\param dest destination path in archive file. it is file path of \a src in archive.
 */
bool Writer::addEntry(const QString & src, const QString& dest)
{
	FileInfo info;
	info.path = QDir::cleanPath(dest);
	if(info.path.startsWith('/') || info.path.startsWith('.') || m_entryList.contains(info))
		return false;

	QFileInfo fileInfo(src);
	if (!fileInfo.exists())
		return false;

	info.src = QDir::cleanPath(src);

	if (fileInfo.isDir() && !fileInfo.isSymLink()) {
		info.type = EN_DIR;
		QDir dir(info.src);
		QStringList entries = dir.entryList(QDir::Dirs | QDir::Files|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden);
		if (!entries.isEmpty()) {
			return addEntries(info.src, entries, dest);
		}
		else {
			m_entryList.append(info);
		}
	}
	else if (fileInfo.isFile()) {
		info.type = EN_FILE;
		info.size = fileInfo.size();
		m_entryList.append(info);
	}
	else {
		return false;
	}
	m_sortFlag = false;
	return true;
}

/*!
	Add multi files or directories to \a dest that is path in archive.
	\a base is base path of srcList. and srcList point to target files or directories by relative to \a base.

	\param base base path of \a srcList.
	\param target file pathes. it is relative path from \a base.
	\param dest destination directory path in archive. all of \a srcList path add to under this entry.
 */
bool Writer::addEntries(const QString & base, const QStringList & srcList, const QString & dest)
{
	bool result = false;
	QString basePath,destPath;
	if (base.isEmpty() || base.endsWith('/')) {
		basePath = base;
	}
	else {
		basePath = base + "/";
	}
	if (dest.isEmpty() || dest.endsWith('/')) {
		destPath = dest;
	}
	else {
		destPath = dest + "/";
	}

	for (auto& src : srcList) {
		if (addEntry(basePath + src, destPath + src)) {
			result = true;
		}
	}

	return result;
}

/*!
	Create archive file by added entries.
	You can select archive format by \a type parameter.

	\param archive file pathl. this function create archive with this file path.
	\param type archive type.
 */
bool Writer::archive(const QString & dest, ArchiveType type)
{
	QtArchiveInfo arInfo;
	if (!open_write_archvie(&arInfo, dest, type)) {
		return false;
	}

	archive_entry* entry;
	for (auto& info : m_entryList) {
		QFileInfo fileInfo(info.src);
		if (!fileInfo.isFile() && !fileInfo.isDir()) continue;

		entry = archive_entry_new();
		archive_entry_set_pathname(entry, info.path.toLocal8Bit());
		if (fileInfo.isDir()) {
			archive_entry_set_filetype(entry, AE_IFDIR);
		}
		else if (fileInfo.isFile()) {
			archive_entry_set_filetype(entry, AE_IFREG);
			archive_entry_set_perm(entry, 0644);
			archive_entry_set_size(entry, fileInfo.size());
			archive_entry_set_birthtime(entry, fileInfo.birthTime().toSecsSinceEpoch(), 0);
			archive_entry_set_mtime(entry, fileInfo.lastModified().toSecsSinceEpoch(), 0);
		}
		
		archive_write_header(arInfo.ar, entry);

		if (fileInfo.isFile() && fileInfo.size() > 0) {
			QFile file(info.src);
			if (file.open(QFile::ReadOnly)) {
				qint64 length = 0;
				while ((length = file.read(arInfo.res.buffer, arInfo.res.buffer_size))>0) {
					archive_write_data(arInfo.ar, arInfo.res.buffer, length);
				}
			}
			file.close();
		}

		archive_entry_free(entry);
	}

	close_write_archive(&arInfo);
	return true;
}

}
