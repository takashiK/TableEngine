#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QFile>

namespace TeArchive {

enum EntryType {
	EN_NONE,
	EN_FILE,
	EN_DIR,
};

struct FileInfo {
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

	EntryType type;
	QString   path;
	QString   src;
	qint64    size;
	QDateTime lastModifyed;
};

enum ArchiveType {
	AR_ZIP,
	AR_7ZIP,
	AR_TAR_GZIP,
	AR_TAR_BZIP2,
};

class Reader :
	public QObject
{
	Q_OBJECT

public:
	Reader(QObject *parent = Q_NULLPTR);
	Reader(const QString& path, QObject *parent = Q_NULLPTR);
	virtual ~Reader();

	void setCallback( bool(*overwrite)(QFile*) );
	bool open( const QString& path);
	void release();

	bool extractAll(const QString& destPath);
	bool extract( const QString& destPath, const QString& base, const QStringList& entries);

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
	bool(*overwrite_check)(QFile* ofile);
};

class Writer :
	public QObject
{
	Q_OBJECT

public:
	Writer(QObject *parent = Q_NULLPTR);
	virtual ~Writer();

	void clear();

	bool addEntry(const QString& src, const QString& dest);
	bool addEntries(const QString& base, const QStringList& srcList, const QString& dest);

	bool archive(const QString& dest, ArchiveType type);
private:
	QList<FileInfo> m_entryList;
	bool m_sortFlag;
};
}