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

#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <cstdint>

#include "TestFileCreator.h"
#include "TeFileInfo.h"
#include <QDir>
#include <QRegularExpression>

/*!
	Create file tree to \a dest by \a paths.
	Default file include path string data.
	If you set positive value to \a extend_mbytes then files append extra extend random data.

	\param dest          Destination  path.
	\param paths         list of file tree entry.
	\param extend_mbytes extended data size (MB unit).
*/
void createFileTree(const QString & dest, const QStringList & paths, int extend_mbytes)
{
	QDir dir(dest);
	dir.mkpath(".");

	QRegularExpression  rootDir = QRegularExpression("^/+");


	for (auto path : paths) {
		path = QDir::cleanPath(path);
		path.replace(rootDir,"");

		int pos = path.lastIndexOf('/');
		if (pos < path.lastIndexOf('.')) {
			//File
			if (pos > 0) {
				dir.mkpath(path.left(pos));
			}

			QFile file(dir.filePath(path));
			file.open(QFile::WriteOnly | QFile::Truncate);
			QByteArray data = path.toLocal8Bit();
			file.write(data);
			if (extend_mbytes > 0) {
				QByteArray extend;
				std::mt19937_64 ran(qHash(path));
				std::uint64_t value;
				for (int i = 0; i < 128*1024; i++) {
					value = ran();
					extend.append(reinterpret_cast<char*>(&value),8);
				}
				for (int i = 0; i < extend_mbytes; i++) {
					file.write(extend);
				}
			}
			file.close();
		}
		else {
			//Directory
			dir.mkpath(path);
		}
	}
}

/*!
	remove file tree.

	\param remove target path.
*/
void cleanFileTree(const QString & path)
{
	QDir dir(path);
	dir.removeRecursively();
}

/*!
	Compare files src to dst.
	Comparition property is below.
	\li file size
	\li internal data if binComp is true.

	\param src src file
	\param dst dst file
	\param binComp If this flag true then process binary compare.

	\return return true if src and dst are same file.
*/
bool compareFileInfo(const QFileInfo& src, const QFileInfo& dst, bool binComp)
{
	bool result = true;
	if (src.size() != dst.size()) {
		EXPECT_EQ(src.size(), dst.size()) << "Different Size: " << src.fileName().toStdString();
		result = false;
	}
	else if(binComp){
		QFile sfile(src.filePath());
		QFile dfile(dst.filePath());
		sfile.open(QFile::ReadOnly);
		dfile.open(QFile::ReadOnly);
		for (;;) {
			QByteArray srcBin = sfile.read(1024);
			QByteArray dstBin = dfile.read(1024);
			if (srcBin.isEmpty() || dstBin.isEmpty()) {
				break;
			}
			else if(srcBin != dstBin){
				ADD_FAILURE() << "Different data: " << src.fileName().toStdString();
				return false;
			}
		}
	}
	return result;
}

/*!
	Compare file tree 
*/
bool compareFileTree(const QString & src, const QString & dst, bool binComp)
{
	bool result = true;
	QDir srcDir(src);
	QDir dstDir(dst);

	if (!srcDir.exists() || !dstDir.exists()) {
		return false;
	}
	QDir::Filters filter = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
	QFileInfoList srcInfo = srcDir.entryInfoList(filter, QDir::Name);
	QFileInfoList dstInfo = dstDir.entryInfoList(filter, QDir::Name);

	EXPECT_EQ(srcInfo.count(), dstInfo.count()) << "Invalid entry count: " << src.toStdString();
	auto srcItr = srcInfo.begin();
	auto dstItr = dstInfo.begin();

	while ((srcItr != srcInfo.end()) && (dstItr != dstInfo.end())) {
		int c = QString::compare(srcItr->fileName(),dstItr->fileName());
		if (c == 0) {
			// src == dst
			if (srcItr->isDir() && dstItr->isDir()) {
				bool res = compareFileTree(srcItr->filePath(), dstItr->filePath(), binComp);
				result = result && res;
			}
			else if (srcItr->isFile() && dstItr->isFile()) {
				bool res = compareFileInfo(*srcItr, *dstItr, binComp);
				result = result && res;
			}
			else {
				EXPECT_EQ(srcItr->isFile(), dstItr->isFile()) << "Invalid Type:" << srcItr->fileName().toStdString();
				result = false;
			}
			++srcItr;
			++dstItr;
		}
		else if (c > 0) {
			// src > dst
			ADD_FAILURE() << "Unexpected Entry: " << dstItr->fileName().toStdString();
			result = false;
			++dstItr;
		}
		else if (c < 0) {
			// src < dst
			ADD_FAILURE() << "Lost Entry: " << srcItr->fileName().toStdString();
			result = false;
			++srcItr;
		}
	}

	while (srcItr != srcInfo.end()) {
		ADD_FAILURE() << "Lost Entry: " << srcItr->fileName().toStdString();
		result = false;
		++srcItr;
	}

	while (dstItr != dstInfo.end()) {
		ADD_FAILURE() << "Unexpected Entry: " << dstItr->fileName().toStdString();
		result = false;
		++dstItr;
	}
	return result;
}

QStandardItem* findChild(QStandardItem* parent, const QString& name)
{
	for (int i = 0; i < parent->rowCount(); i++) {
		QStandardItem* item = parent->child(i);
		if (item->data(Qt::DisplayRole).toString() == name) {
			return item;
		}
	}
	return nullptr;
}

void expectEntries(QStandardItem* root, const QStringList& paths, const QDateTime& date)
{
	for (auto& path : paths) {
		QStandardItem* parent = root;
		QStringList spath = path.split('/');
		QString last = spath.takeLast();

		//middle path loop. so all entries are Directory.
		for (auto& name : spath) {
			QStandardItem* entry = findChild(parent,name);
			if (entry == nullptr) {
				TeFileInfo tInfo;
				tInfo.type = TeFileInfo::EN_DIR;
				tInfo.path = parent->data(ROLE_FINFO_PATH).toString() + "/" + name;
				entry = new QStandardItem();
				convertToStandardItem(tInfo, entry);
				parent->appendRow(entry);
			}
			parent = entry;
		}

		//last path entry. it's file if name is valid.
		if (!last.isEmpty()) {
			TeFileInfo tInfo;
			tInfo.type = TeFileInfo::EN_FILE;
			tInfo.path = parent->data(ROLE_FINFO_PATH).toString() + "/" + last;
			QStandardItem* entry = new QStandardItem();
			convertToStandardItem(tInfo, entry);
			parent->appendRow(entry);
		}
	}
}

bool lessThanFileInfo(const QStandardItem* l, const QStandardItem* r) {
	if (l->data(Qt::DisplayRole).toString() == "..") return true;
	if (r->data(Qt::DisplayRole).toString() == "..") return false;

	return l->data(Qt::DisplayRole).toString() < r->data(Qt::DisplayRole).toString();
}

bool compareFileEntry(const QStandardItem* src, const QFileInfo& dst, bool binComp)
{
	bool result = true;
	TeFileInfo info;
	convertToFileInfo(src, &info);
	if (info.size != dst.size()) {
		EXPECT_EQ(info.size, dst.size()) << "Different Size: " << info.path.toStdString();
		result = false;
	}
	else if(binComp){
		QFile dfile(dst.filePath());
		dfile.open(QFile::ReadOnly);
		QByteArray dstBin = dfile.readAll();
		if (src->data(ROLE_FINFO_PATH).toString() != dstBin) {
			ADD_FAILURE() << "Different data: " << src->data(ROLE_FINFO_PATH).toString().toStdString();
			result = false;
		}
	}
	return result;
}

bool lessThanFileItem(const QStandardItem* l, const QStandardItem* r) {
	if (l->data(Qt::DisplayRole).toString() == "..") return true;
	if (r->data(Qt::DisplayRole).toString() == "..") return false;

	return l->data(ROLE_FINFO_PATH).toString() < r->data(ROLE_FINFO_PATH).toString();
}

bool compareFileTree(const QStandardItem* srcRoot, const QString & dst, bool binComp)
{
	bool result = true;
	QDir dstDir(dst);

	if (!dstDir.exists()) {
		return false;
	}
	QDir::Filters filter = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
	QFileInfoList dstInfo = dstDir.entryInfoList(filter, QDir::Name);

	EXPECT_EQ(srcRoot->rowCount(), dstInfo.count()) << "Invalid entry count: " << srcRoot->data(ROLE_FINFO_PATH).toString().toStdString();
	QList<QStandardItem*> children;
	for(int i=0; i< srcRoot->rowCount(); i++){
		children.append(srcRoot->child(i));
	}
	std::sort(children.begin(), children.end(), lessThanFileItem);
	auto srcItr = children.begin();
	auto dstItr = dstInfo.begin();

	while ((srcItr != children.end()) && (dstItr != dstInfo.end())) {
		QStandardItem* src = *srcItr;
		int c = QString::compare(src->data(Qt::DisplayRole).toString(), dstItr->fileName());
		if (c == 0) {
			// src == dst
			if ((src->data(ROLE_FINFO_TYPE).toInt() == TeFileInfo::EN_DIR) && dstItr->isDir()) {
				bool res = compareFileTree(src, dstItr->filePath(), binComp);
				result = result && res;
			}if ((src->data(ROLE_FINFO_TYPE).toInt() == TeFileInfo::EN_FILE) && dstItr->isFile()) {
				bool res = compareFileEntry(src, *dstItr, binComp);
				result = result && res;
			}
			else {
				EXPECT_EQ(src->data(ROLE_FINFO_TYPE).toInt() == TeFileInfo::EN_FILE, dstItr->isFile()) << "Invalid Type:" << src->data(ROLE_FINFO_PATH).toString().toStdString();
				result = false;
			}
			++srcItr;
			++dstItr;
		}
		else if (c > 0) {
			// src > dst
			ADD_FAILURE() << "Unexpected Entry: " << dstItr->fileName().toStdString();
			result = false;
			++dstItr;
		}
		else if (c < 0) {
			// src < dst
			ADD_FAILURE() << "Lost Entry: " << src->data(ROLE_FINFO_PATH).toString().toStdString();
			result = false;
			++srcItr;
		}
	}

	while (srcItr != children.end()) {
		QStandardItem* src = *srcItr;
		ADD_FAILURE() << "Lost Entry: " << src->data(ROLE_FINFO_PATH).toString().toStdString();
		result = false;
		++srcItr;
	}

	while (dstItr != dstInfo.end()) {
		ADD_FAILURE() << "Unexpected Entry: " << dstItr->fileName().toStdString();
		result = false;
		++dstItr;
	}

	return result;
}

void convertToStandardItem(const TeFileInfo& info, QStandardItem* item)
{
	QFileInfo finfo(info.path);
	item->setData(finfo.fileName(), Qt::DisplayRole);
	item->setData(info.type, ROLE_FINFO_TYPE);
	item->setData(info.path, ROLE_FINFO_PATH);
	item->setData(info.size, ROLE_FINFO_SIZE);
	item->setData(info.lastModified, ROLE_FINFO_DATE);
	item->setData(info.permissions, ROLE_FINFO_PERM);
}

void convertToFileInfo(const QStandardItem* item, TeFileInfo* info)
{
	info->type = static_cast<TeFileInfo::EntryType>(item->data(ROLE_FINFO_TYPE).toInt());
	info->path = item->data(ROLE_FINFO_PATH).toString();
	info->size = item->data(ROLE_FINFO_SIZE).toLongLong();
	info->lastModified = item->data(ROLE_FINFO_DATE).toDateTime();
	info->permissions = item->data(ROLE_FINFO_PERM).toUInt();
}

