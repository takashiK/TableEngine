#include "TeFileInfo.h"
#include <QFileInfo>
#include <QStandardItem>
#include <QFileIconProvider>
TeFileInfo::TeFileInfo()
{
	type = EN_NONE;
	size = 0;
	permissions = 0x644;
}

TeFileInfo::~TeFileInfo()
{}

QList<QStandardItem*> TeFileInfo::exportItem(bool filesystem, bool bColum) const
{
	QList<QStandardItem*> items;
	QFileInfo finfo(path);

	//COL_NAME
	QStandardItem* item = nullptr;
	QFileIconProvider iconProvider;
	if (filesystem && finfo.exists()) {
		item = new QStandardItem(iconProvider.icon(finfo), finfo.fileName());
	}
	else {
		switch(type){
		case EN_DIR:
			item = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), finfo.fileName());
			break;
		case EN_FILE:
			item = new QStandardItem(iconProvider.icon(QFileIconProvider::File), finfo.fileName());
			break;
		default:
			item = new QStandardItem(finfo.fileName());
			break;
		}
	}
	item->setData(path, ROLE_PATH);
	item->setData(finfo.fileName(), ROLE_NAME);
	item->setData(permissions, ROLE_PERM);
	item->setData(type, ROLE_TYPE);
	item->setData(size, ROLE_SIZE);
	item->setData(lastModified, ROLE_DATE);
	items.append(item);

	//COL_SIZE
	if (type == EN_FILE) {
		item = new QStandardItem(QLocale::system().formattedDataSize(size));
	}
	else {
		item = new QStandardItem("");
	}
	items.append(item);

	//COL_TYPE
	switch(type){
		case EN_DIR:
		item = new QStandardItem(QObject::tr("Folder"));
		break;
	case EN_FILE:
		item = new QStandardItem(finfo.suffix() + " " + QObject::tr("File"));
		break;
	default:
		item = new QStandardItem("");
		break;
	}
	items.append(item);

	//COL_DATE
	item = new QStandardItem(QLocale::system().toString(lastModified, QLocale::ShortFormat));
	items.append(item);

	return items;
}

void TeFileInfo::exportToItem(QStandardItem * parent, bool bColum) const
{
	parent->appendRow(exportItem(bColum));
}

void TeFileInfo::importFromItem(const QStandardItem* item)
{
	path = item->data(ROLE_PATH).toString();
	permissions = item->data(ROLE_PERM).toUInt();
	type = (EntryType)item->data(ROLE_TYPE).toInt();
	size = item->data(ROLE_SIZE).toULongLong();
	lastModified = item->data(ROLE_DATE).toDateTime();
}
