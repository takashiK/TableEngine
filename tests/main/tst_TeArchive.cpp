/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
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

/*
 * TeArchive::Writer
 *    - void clear();
 *      - before state
 *          - empty
 *          - any entries
 *          - call clear twice.
 *    - bool addEntry(const QString& src, const QString& dest);
 *      - variation of path
 *          - empty
 *          - null
 *          - root directory
 *          - absolute path
 *          - relative path
 *          - path without directories
 *          - path with directories
 *          - unacceptable charactor code
 *          - invalid separator
 *          - directory path is terminated by separator
 *          - directory path is not terminated by separator
 *          - path with "." , ".." , "..."
 *          - path start with "." , ".." , "..."
 *          - src file not found.
 *          - point to hidden file.
 *          - point to system file.
 *          - directory children that hidden and system types.
 *      - variation of condition
 *          - add entry that already added. (for same type)
 *          - add entry that already added. (for different type)
 *          - add directory that have children.
 *          - add directory that don't have children.
 *    - bool addEntries(const QString& base, const QStringList& srcList, const QString& dest=QString());
 *      - variation of path and condition is same as addEntry
 *      - variation of srcList
 *          - single type of path variation.
 *          - mixed path variation.
 *    - bool archive(const QString& dest, ArchiveType type);
 *      - variation of path and condition is same as addEntry
 *      - existance of dest path
 *          - exist and valid
 *          - already file exist
 *          - already exist but it is directory
 *          - directory is not exist
 *      - variation of ArchiveType
 *          - Zip
 *          - 7zip
 *          - tar + gzip
 *          - tar + bzip2
 *      - condition of entries
 *          - empty
 *          - empty caused by clean
 *          - single valid entry variation of path that same as addEntry
 *          - single invalid entry
 *          - multi valid entry
 *          - mixed entry valid and invalid
 * TeArchive::Reader
 *    - void setCallback( bool(*overwrite)(QFile*) );
 *      - variation of operation
 *          - don't change path
 *          - change path to valid path
 *          - change path to null
 *          - change path to invalid path
 *    - bool open( const QString& path);
 *      - path variation.
 *          - zip file
 *          - 7zip file
 *          - tar gzip
 *          - tar bzip2
 *          - other of libarchive supported files.
 *          - null
 *          - directory
 *          - normal file
 *          - empty file
 *    - ArchiveType type();
 *       - variation of archive type
 *    - void close();
 *       - operation order
 *          - after open.
 *          - before open.
 *          - call close twice.
 *    - const QString& path()
 *       - operation order
 *          - before open
 *          - after open
 *          - after close
 *    - bool extractAll(const QString& destPath);
 *       - operation order
 *          - before open
 *          - after open
 *          - after close
 *       - variation of open
 *       - path variation of included files
 *          - valid data
 *          - include absolute path
 *          - include ".."
 *          - include ".." that path is target to upper directory of destination path.
 *    - bool extract( const QString& destPath, const QString& base, const QStringList& entries);
 *       - same test of extractAll
 *       - invalid base path
 *       - variation of entries
 *          - valid entry
 *          - invalid entry
 *          - mixed entry
 *    - TeArchive::Reader::iterator
 *       - check included files
 */

#include <gmock/gmock.h>

#include <TeArchive.h>
#include <test_util/TestFileCreator.h>
#include <test_util/FileEntry.h>

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QThread>

/*!
	Check "archive" and "extraction". check method is below.
	
	1) Create file tree by \a setup list to debug/test directory.
	2) Archive file tree of 1) by TeArchive::Writer. its archive type decide by \a type.
	3) Extract from archive file to debug/test2 directory by TeArchive::Reader.
	4) Compair debug/test and debug/test2.
	5) Confirm debug/test2 to \q expect.
 */
bool checkArchive(const QStringList& expect, const QStringList& setup, TeArchive::ArchiveType type)
{
	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	createFileTree("debug/test", expect);

	FileEntry entries(nullptr, "root");
	expectEntries(&entries, expect, QDateTime(), true);

	TeArchive::Writer writer;
	TeArchive::Reader reader;

	writer.addEntries("debug/test", setup, "");
	EXPECT_TRUE(writer.archive("debug/test.zip", type));

	EXPECT_TRUE(reader.open("debug/test.zip"));

	EXPECT_EQ(type, reader.type());

	QDir dir;
	dir.mkpath("debug/test2");
	reader.extractAll("debug/test2");
	reader.close();

	bool r1 = compareFileTree("debug/test", "debug/test2", true);
	bool r2 = compareFileTree(&entries, "debug/test2", true);

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	return (r1 && r2);
}

/*!
	Check archive with flat file list.
*/
TEST(tst_TeArchive, archive_flatfile_zip)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	
	checkArchive(list, list, TeArchive::AR_ZIP);
	checkArchive(list, list, TeArchive::AR_7ZIP);
	checkArchive(list, list, TeArchive::AR_TAR_GZIP);
	checkArchive(list, list, TeArchive::AR_TAR_BZIP2);
}

/*!
	Check archive files with directories.
*/
TEST(tst_TeArchive, archive_rankfile_zip)
{
	QStringList list;
	list.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	list.append("dir3/dir3_2/file3_2_1.txt");
	list.append("dir1/file1_2.txt");
	list.append("dir1/file1_1.txt");
	list.append("dir2");

	checkArchive(list, list, TeArchive::AR_ZIP);
	checkArchive(list, list, TeArchive::AR_7ZIP);
	checkArchive(list, list, TeArchive::AR_TAR_GZIP);
	checkArchive(list, list, TeArchive::AR_TAR_BZIP2);
}

/*!
	Check excption invalid entries.
*/
TEST(tst_TeArchive, archive_write_invalid_entry)
{
	cleanFileTree("debug/test");
	QStringList list;
	list.append("file1.txt");
	createFileTree("debug/test", list);

	TeArchive::Writer writer;

	EXPECT_FALSE(writer.addEntry(QString(), ""));
	EXPECT_FALSE(writer.addEntry("",""));
	EXPECT_FALSE(writer.addEntry("not_exist_file", ""));

	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", ".."));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "..."));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "/test"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "test/../.."));

	EXPECT_TRUE(writer.addEntry("debug/test/file1.txt", "file1.txt"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "file1.txt"));
	EXPECT_FALSE(writer.addEntry("debug/test", "file1.txt"));
#if 0
	//Invalid Path string in windows.
	//checker not support yet.
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", ":"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "*"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "?"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "\""));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "|"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", "<"));
	EXPECT_FALSE(writer.addEntry("debug/test/file1.txt", ">"));
#endif

	QStringList badlist;
	badlist.append("not_exist_file1");
	badlist.append("not_exist_file2");
	EXPECT_FALSE(writer.addEntries("debug/test",badlist,"multi"));

	badlist.append("file1.txt");
	int count = writer.count();
	EXPECT_TRUE(writer.addEntries("debug/test", badlist, "multi"));
	EXPECT_EQ(count + 1, writer.count());


	cleanFileTree("debug/test");
}

/*!
	Check unusual but acceptable entries.
*/
TEST(tst_TeArchive, archive_write_valid_entry)
{
	cleanFileTree("debug/test");
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	createFileTree("debug/test", list);

	TeArchive::Writer writer;
	EXPECT_TRUE(writer.addEntry("debug/test/file1.txt", "test1/test/.."));
	EXPECT_TRUE(writer.addEntry("debug/test/file1.txt", "test2/test/..."));
	EXPECT_TRUE(writer.addEntry("debug/test/file1.txt", "test3/../test/../file3"));
	EXPECT_TRUE(writer.addEntry("debug/test/file1.txt", "test4//test"));
	EXPECT_TRUE(writer.addEntry("debug/test", "test7"));

	writer.clear();
	EXPECT_EQ(0, writer.count());
	writer.clear();
	EXPECT_EQ(0, writer.count());

	cleanFileTree("debug/test");
}

namespace {

	bool noCallCheck(QFileInfo* info)
	{
		ADD_FAILURE();
		return false;
	}

	bool unAccept(QFileInfo* info)
	{
		return false;
	}

	bool overwrite(QFileInfo* info)
	{
		return true;
	}

	bool rename(QFileInfo* info)
	{
		info->setFile(info->filePath() + "_");
		return true;
	}

}

/*!
	Check variation of callback function.
	case 1 : no collision so it is not call callback function.
	case 2 : occur collision but unaccept all of cases.
	case 3 : occur collision and overwrite all of cases.
	case 4 : occur collision and rename it.
*/
TEST(tst_TeArchive, archive_readCallback)
{
	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QFile file;
	QStringList list;
	list.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	list.append("dir3/dir3_2/file3_2_1.txt");
	list.append("dir1/file1_2.txt");
	list.append("dir1/file1_1.txt");
	list.append("dir2");
	createFileTree("debug/test", list);

	TeArchive::Writer writer;
	writer.addEntry("debug/test", "");
	writer.archive("debug/test.zip", TeArchive::AR_ZIP);

	TeArchive::Reader reader;
	reader.open("debug/test.zip");
	EXPECT_EQ("debug/test.zip", reader.path());

	//normal 
	reader.setCallback(noCallCheck);
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	//unaccept overwrite
	file.setFileName("debug/test/dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	file.open(QFile::WriteOnly | QFile::Truncate);
	file.write("test");
	file.close();

	file.setFileName("debug/test2/dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	file.open(QFile::WriteOnly | QFile::Truncate);
	file.write("test");
	file.close();

	reader.setCallback(unAccept);
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	//overwrite
	createFileTree("debug/test", list);

	file.setFileName("debug/test2/dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	file.open(QFile::WriteOnly|QFile::Truncate);
	file.write("test");
	file.close();
	file.setFileName("debug/test2/dir1/file1_1.txt");
	file.open(QFile::WriteOnly | QFile::Truncate);
	file.write("test");
	file.close();
	file.setFileName("debug/test2/dir1/file1_2.txt");
	file.open(QFile::WriteOnly | QFile::Truncate);
	file.write("test");
	file.close();

	reader.setCallback(overwrite);
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	//changefilename
	QFile::copy("debug/test/" + list[0], "debug/test/" + list[0] + "_");
	QFile::copy("debug/test/" + list[1], "debug/test/" + list[1] + "_");
	QFile::copy("debug/test/" + list[2], "debug/test/" + list[2] + "_");
	QFile::copy("debug/test/" + list[3], "debug/test/" + list[3] + "_");

	reader.setCallback(rename);
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}

/*!
	check partial entry extraction.
*/
TEST(tst_TeArchive, archive_read_partial)
{
	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QStringList list;
	list.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	list.append("dir3/dir3_2/file3_2_1.txt");
	list.append("dir1/file1_2.txt");
	list.append("dir1/file1_1.txt");
	list.append("dir2");
	createFileTree("debug/test", list);

	TeArchive::Writer writer;
	writer.addEntry("debug/test", "");
	writer.archive("debug/test.zip", TeArchive::AR_ZIP);

	TeArchive::Reader reader;
	reader.open("debug/test.zip");
	reader.extract("debug/test2", "", list);
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}

/*!
	progress bar test.
*/
TEST(tst_TeArchive, archive_progress)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	createFileTree("debug/test", list);

	QThread thread;
	TeArchive::Writer writer;


}