#pragma once

#include <QObject>
#include <QDateTime>

/**
 * @file TeFileInfo.h
 * @brief Data transfer object for a single file or directory entry.
 * @ingroup utility
 */

class QStandardItem;

/**
 * @class TeFileInfo
 * @brief POD-like container for the attributes of one file or directory entry.
 * @ingroup utility
 *
 * @details Used by TeArchive::Reader, TeFinder, and the archive folder view
 * as a unified representation of any file-system or archive entry.  Can be
 * exported to / imported from a QStandardItem row for use in
 * QStandardItemModel-backed views.
 *
 * @see TeArchive::Reader, TeFinder, TeArchiveFolderView
 */
class TeFileInfo
{
	Q_GADGET
public:
	/** @brief Type of the directory entry. */
	enum EntryType {
		EN_NONE,   ///< Undefined type.
		EN_FILE,   ///< Regular file.
		EN_DIR,    ///< Directory.
		EN_PARENT, ///< Synthetic parent-directory entry ("..").
	};
	Q_ENUM(EntryType)

	/** @brief Custom Qt model roles used to store TeFileInfo fields in a QStandardItem. */
	enum ItemRole {
		ROLE_PATH = Qt::UserRole + 1, ///< Absolute path string.
		ROLE_NAME,                    ///< Display name.
		ROLE_PERM,                    ///< File permissions bitmask.
		ROLE_TYPE,                    ///< EntryType value.
		ROLE_SIZE,                    ///< File size in bytes.
		ROLE_DATE,                    ///< Last-modified QDateTime.
		ROLE_ACSR,                    ///< Attached TeFileInfoAcsr pointer.
		ROLE_USER_START,              ///< First role available for subclass use.
	};

	/** @brief Column indices used in multi-column item models. */
	enum ItemColumn {
		COL_NAME = 0,  ///< File name column.
		COL_SIZE,      ///< File size column.
		COL_TYPE,      ///< Entry type column.
		COL_DATE,      ///< Last-modified date column.
		COL_USER_START ///< First column available for subclass use.
	};

	Q_PROPERTY(EntryType type MEMBER type)
	Q_PROPERTY(QString path MEMBER path)
	Q_PROPERTY(qint64 size MEMBER size)
	Q_PROPERTY(QDateTime lastModified MEMBER lastModified)
	Q_PROPERTY(int permission MEMBER permissions)

public:
	TeFileInfo();
	~TeFileInfo();

	/**
	 * @brief Exports this entry as a list of QStandardItem columns.
	 * @param filesystem true to also set filesystem-style decoration roles.
	 * @param bColum     true to generate all columns (COL_*); false for name only.
	 * @return Row of QStandardItem objects suitable for appendRow().
	 */
	QList<QStandardItem*> exportItem(bool filesystem=true, bool bColum = true) const;

	/**
	 * @brief Writes this entry's data into @p parent's custom roles.
	 * @param parent QStandardItem to receive the data.
	 * @param bColum true to also append child column items.
	 */
	void exportToItem(QStandardItem* parent, bool bColum = true) const;

	/**
	 * @brief Reads this entry's data from @p item's custom roles.
	 * @param item QStandardItem to read from.
	 */
	void importFromItem(const QStandardItem* item);

	EntryType type;         ///< Entry type descriptor.
	QString   path;         ///< Full path of the entry.
	qint64    size;         ///< File size in bytes.
	QDateTime lastModified; ///< Last modification time.
	int       permissions;  ///< File permission bitmask.
};

