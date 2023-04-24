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
#include "FileEntry.h"
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

void expectEntries(FileEntry* root, const QStringList& paths, const QDateTime& date, bool noDotDot)
{
	for (auto& path : paths) {
		FileEntry* parent = root;
		QStringList spath = path.split('/');
		for (auto& name : spath) {
			FileEntry* entry = parent->findChild<FileEntry*>(name);
			if (entry == nullptr) {
				if (name.indexOf(".") > 0) {
					entry = new FileEntry(parent, name, path.size(), date, path);
				}
				else {
					entry = new FileEntry(parent, name);
					if (!noDotDot) {
						FileEntry* pp = new FileEntry(entry, "..");
						pp->setObjectName("..");
					}
				}
				entry->setObjectName(name);
			}
			parent = entry;
		}
	}
}

bool lessThanFileEntryObject(const QObject* l, const QObject* r) {
	if (l->objectName() == "..") return true;
	if (r->objectName() == "..") return false;

	return l->objectName() < r->objectName();
}

bool compareFileEntry(const FileEntry* src, const QFileInfo& dst, bool binComp)
{
	bool result = true;
	if (src->size != dst.size()) {
		EXPECT_EQ(src->size, dst.size()) << "Different Size: " << src->path.toStdString();
		result = false;
	}
	else if(binComp){
		QFile dfile(dst.filePath());
		dfile.open(QFile::ReadOnly);
		QByteArray dstBin = dfile.readAll();
		if (src->src != dstBin) {
			ADD_FAILURE() << "Different data: " << src->path.toStdString();
			result = false;
		}
	}
	return result;
}

bool compareFileTree(const FileEntry* srcRoot, const QString & dst, bool binComp)
{
	bool result = true;
	QDir dstDir(dst);

	if (!dstDir.exists()) {
		return false;
	}
	QDir::Filters filter = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
	QFileInfoList dstInfo = dstDir.entryInfoList(filter, QDir::Name);

	EXPECT_EQ(srcRoot->children().count(), dstInfo.count()) << "Invalid entry count: " << srcRoot->path.toStdString();
	QObjectList children = srcRoot->children();
	std::sort(children.begin(), children.end(), lessThanFileEntryObject);
	auto srcItr = children.begin();
	auto dstItr = dstInfo.begin();

	while ((srcItr != children.end()) && (dstItr != dstInfo.end())) {
		FileEntry* src = qobject_cast<FileEntry*>(*srcItr);
		int c = QString::compare(src->path, dstItr->fileName());
		if (c == 0) {
			// src == dst
			if ((src->type == TeArchive::EN_DIR) && dstItr->isDir()) {
				bool res = compareFileTree(src, dstItr->filePath(), binComp);
				result = result && res;
			}if ((src->type == TeArchive::EN_FILE) && dstItr->isFile()) {
				bool res = compareFileEntry(src, *dstItr, binComp);
				result = result && res;
			}
			else {
				EXPECT_EQ(src->type == TeArchive::EN_FILE, dstItr->isFile()) << "Invalid Type:" << src->path.toStdString();
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
			ADD_FAILURE() << "Lost Entry: " << src->path.toStdString();
			result = false;
			++srcItr;
		}
	}

	while (srcItr != children.end()) {
		FileEntry* src = qobject_cast<FileEntry*>(*srcItr);
		ADD_FAILURE() << "Lost Entry: " << src->path.toStdString();
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

