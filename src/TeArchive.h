#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>

namespace TeArchive {

enum EntryType {
	EN_NONE,
	EN_FILE,
	EN_DIR,
};

struct FileInfo {
	FileInfo() :type(EN_NONE), size(0) {}

	EntryType type;
	QString   path;
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

	void open( const QString& path);
	void release();

	bool extractAll(const QString& destPath);
	bool extract( const QString& destPath, const QString& base, const QStringList& entries, bool recursive=true);

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
		bool operator!=(const const_iterator &o) const Q_DECL_NOTHROW { return (data != o.data) || (info.path != o.info.path); }
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
}
