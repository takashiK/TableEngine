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
#include "TeArchive.h"
#include "archive.h"
#include "archive_entry.h"

#include <QFile>
#include <QDir>
#include <algorithm>

namespace TeArchive{
namespace {

	struct QtArchiveStatus
	{
		QFile file;
		static const int buffer_size = 1024 * 128;
		char  buffer[buffer_size];
	};

	struct QtArchiveInfo
	{
		QtArchiveInfo() : ar(Q_NULLPTR), entry(Q_NULLPTR){}
		struct archive *ar;
		struct archive_entry *entry;
		QtArchiveStatus st;
	};

	extern "C" int qt_read_open_callback(struct archive *a, void *client_data)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
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

	extern "C" int qt_close_callback(struct archive *a, void *client_data)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);

		qas->file.close();
		return ARCHIVE_OK;
	}

	extern "C" la_ssize_t qt_read_callback(struct archive *a, void *client_data, const void **buffer)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
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

	extern "C" la_int64_t qt_skip_callback(struct archive *a, void *client_data, la_int64_t request)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
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

	extern "C" la_int64_t qt_seek_callback(struct archive *a, void *client_data, la_int64_t offset, int whence)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
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

	extern "C" int qt_write_open_callback(struct archive *a, void *client_data)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
		int result = ARCHIVE_OK;

		if (!qas->file.open(QFile::WriteOnly | QFile::Truncate)) {
			result = ARCHIVE_FATAL;
			archive_set_error(a, EINVAL, qas->file.errorString().toUtf8());
		}
		return result;
	}

	extern "C"	la_ssize_t	qt_write_callback(struct archive *a, void *client_data, const void *buffer, size_t length)
	{
		QtArchiveStatus * qas = static_cast<QtArchiveStatus*>(client_data);
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

	bool open_read_archvie(QtArchiveInfo* arInfo, const QString& path) 
	{
		arInfo->ar = archive_read_new();
		archive_read_support_format_all(arInfo->ar);
		archive_read_support_compression_all(arInfo->ar);

		archive_read_set_seek_callback(arInfo->ar, qt_seek_callback);

		arInfo->st.file.setFileName(path.toLocal8Bit());

		int r = archive_read_open2(arInfo->ar, &arInfo->st, qt_read_open_callback, qt_read_callback, qt_skip_callback, qt_close_callback);
		if (r != ARCHIVE_OK) {
			archive_read_close(arInfo->ar);
			archive_read_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
			return false;
		}

		return true;
	}

	void close_read_archive(QtArchiveInfo* arInfo)
	{
		arInfo->entry = Q_NULLPTR;
		if (arInfo->ar != Q_NULLPTR) {
			archive_read_close(arInfo->ar);
			archive_read_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
		}
	}

	bool read_next_entry(QtArchiveInfo* arInfo, FileInfo* info) {
		info->type = EN_NONE;
		info->size = 0;
		info->path.clear();
		info->lastModifyed = QDateTime::currentDateTime();

		while (ARCHIVE_OK == archive_read_next_header(arInfo->ar, &arInfo->entry)) {

			__LA_MODE_T mode = archive_entry_filetype(arInfo->entry);
			switch (mode) {
			case AE_IFREG:
				info->type = EN_FILE;
				break;
			case AE_IFDIR:
				info->type = EN_DIR;
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
				info->type = EN_NONE;
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

	bool copy_data(QtArchiveInfo* arInfo, QFile *ofile)
	{
		int r;
		const void *buff;
		size_t size=0;
		la_int64_t offset=0;

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
		}
	}

	bool open_write_archvie(QtArchiveInfo* arInfo, const QString& path, ArchiveType type)
	{
		arInfo->ar = archive_write_new();

		switch(type){
		case AR_ZIP:
			archive_write_set_format_zip(arInfo->ar);
			break;
		case AR_7ZIP:
			archive_write_set_format_7zip(arInfo->ar);
			archive_write_set_format_option(arInfo->ar, "7zip", "compression", "lzma2");
			break;
		case AR_TAR_GZIP:
			archive_write_set_format_gnutar(arInfo->ar);
			archive_write_set_compression_gzip(arInfo->ar);
			break;
		case AR_TAR_BZIP2:
			archive_write_set_format_gnutar(arInfo->ar);
			archive_write_set_compression_bzip2(arInfo->ar);
			break;
		}

		arInfo->st.file.setFileName(path.toLocal8Bit());

		int r = archive_write_open(arInfo->ar, &arInfo->st, qt_write_open_callback, qt_write_callback, qt_close_callback);
		if (r != ARCHIVE_OK) {
			archive_write_close(arInfo->ar);
			archive_write_free(arInfo->ar);
			arInfo->ar = Q_NULLPTR;
			return false;
		}

		return true;
	}

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

Reader::Reader(QObject *parent)
	: QObject(parent)
{
	overwrite_check = Q_NULLPTR;
}

Reader::Reader(const QString & path, QObject * parent)
	: QObject(parent)
{
	overwrite_check = Q_NULLPTR;
	open(path);
}


Reader::~Reader()
{
}

void Reader::setCallback( bool(*overwrite)(QFileInfo*) )
{
	overwrite_check = overwrite;
}

bool Reader::open(const QString & path)
{
	release();
	if (!QFile::exists(path)) {
		return false;
	}

	QtArchiveInfo arInfo;

	if (!open_read_archvie(&arInfo, path)) {
		return false;
	}

	close_read_archive(&arInfo);
	m_path = path;

	return true;
}

void Reader::release()
{
	m_path.clear();
}

bool Reader::extractAll(const QString & destPath)
{
	FileInfo info;

	if (m_path.isEmpty()) {
		return false;
	}
	
	QtArchiveInfo arInfo;

	if (!open_read_archvie(&arInfo, m_path)) {
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
	return true;
}

bool Reader::extract(const QString & destPath, const QString & base, const QStringList & entries)
{
	FileInfo info;

	if (m_path.isEmpty()) {
		return false;
	}

	QtArchiveInfo arInfo;

	if (!open_read_archvie(&arInfo, m_path)) {
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
	return true;
}

Reader::const_iterator Reader::begin()
{
	return const_iterator(this);
}

Reader::const_iterator Reader::end()
{
	return const_iterator();
}

Reader::const_iterator::const_iterator(Reader * ar)
{
	if (!ar->m_path.isEmpty()) {
		QtArchiveInfo* arInfo = new QtArchiveInfo;
		data = arInfo;

		open_read_archvie(arInfo, ar->m_path);
		if (!read_next_entry(arInfo, &info)) {
			close_read_archive(arInfo);
			delete arInfo;
			data = Q_NULLPTR;
		}
	}
}

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

Writer::Writer(QObject *parent)
	: QObject(parent)
{
	m_sortFlag = true;
}


Writer::~Writer()
{
}

void Writer::clear()
{
	m_entryList.clear();
}

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
		int r = archive_write_header(arInfo.ar, entry);

		if (fileInfo.isFile() && fileInfo.size() > 0) {
			QFile file(info.src);
			if (file.open(QFile::ReadOnly)) {
				qint64 length = 0;
				while ((length = file.read(arInfo.st.buffer, arInfo.st.buffer_size))>0) {
					archive_write_data(arInfo.ar, arInfo.st.buffer, length);
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
