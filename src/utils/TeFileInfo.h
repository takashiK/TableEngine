#pragma once

#include <QObject>
#include <QDateTime>

class QStandardItem;

class TeFileInfo
{
	Q_GADGET
public:
	enum EntryType {
		EN_NONE,	//!< Undefined type
		EN_FILE,	//!< File type
		EN_DIR,		//!< Directory type
		EN_PARENT,	//!< Parent directory type
	};
	Q_ENUM(EntryType)

	enum ItemRole {
		ROLE_PATH = Qt::UserRole + 1,
		ROLE_NAME,
		ROLE_PERM,
		ROLE_TYPE,
		ROLE_SIZE,
		ROLE_DATE,
		ROLE_ACSR,
		ROLE_USER_START,
	};

	enum ItemColumn {
		COL_NAME = 0,
		COL_SIZE,
		COL_TYPE,
		COL_DATE,
		COL_USER_START
	};

		
	Q_PROPERTY(EntryType type MEMBER type)
	Q_PROPERTY(QString path MEMBER path)
	Q_PROPERTY(qint64 size MEMBER size)
	Q_PROPERTY(QDateTime lastModified MEMBER lastModified)
	Q_PROPERTY(int permission MEMBER permissions)

public:
	TeFileInfo();
	~TeFileInfo();

	QList<QStandardItem*> exportItem(bool filesystem=true, bool bColum = true) const;
	void exportToItem(QStandardItem* parent, bool bColum = true) const;
	void importFromItem(const QStandardItem* item);

	EntryType type;			//!< Entry type descripter.
	QString   path;			//!< Path in archive.
	qint64    size;			//!< File size.
	QDateTime lastModified;	//!< File last modification time.
	int       permissions;	//!< File permission.
};

