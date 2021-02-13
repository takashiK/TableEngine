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
#include <test_util/ProgressTracker.h>

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QThread>

#if 0
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

#endif

/*!
	progress bar test.
*/
TEST(tst_TeArchive, archive_progress)
{
	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	createFileTree("debug/test", list, 1);

	TeArchive::Writer writer;
	writer.addEntry("debug/test", "");

	QThread wthread;
	writer.moveToThread(&wthread);
	wthread.start();

	QThread rcvThread;
	ProgressTracker wtracker;
	ProgressTracker* pwTracker = &wtracker;
	ProgressTracker tracker;
	ProgressTracker* pTracker = &tracker;
	tracker.moveToThread(&rcvThread);
	rcvThread.start();

	QObject::connect(&writer, &TeArchive::Writer::maximumValue, &wtracker, &ProgressTracker::setMaxValue);
	QObject::connect(&writer, &TeArchive::Writer::valueChanged, &wtracker, &ProgressTracker::setProgress);
	QObject::connect(&writer, &TeArchive::Writer::currentFileInfoChanged, [pwTracker](const TeArchive::FileInfo & info) {pwTracker->setText(info.path); });

	writer.archive("debug/test.zip", TeArchive::AR_ZIP);

	wthread.quit();
	wthread.wait();

	//Check Progress values
	EXPECT_LE(list.size() *1024,wtracker.maxvalue);
	EXPECT_LE(list.size(), wtracker.count);
	EXPECT_EQ(wtracker.maxvalue, wtracker.progress);
	EXPECT_EQ(list.size(), wtracker.strList.size());
	if (list.size() == wtracker.strList.size()) {
		for (int i = 0; i < wtracker.strList.size(); i++) {
			EXPECT_EQ(list[i].toStdString(), wtracker.strList[i].toStdString());
		}
	}

	TeArchive::Reader reader;
	reader.open("debug/test.zip");

	QThread thread;
	reader.moveToThread(&thread);
	thread.start();

	QObject::connect(&reader, &TeArchive::Reader::maximumValue, &tracker, &ProgressTracker::setMaxValue);
	QObject::connect(&reader, &TeArchive::Reader::valueChanged, &tracker, &ProgressTracker::setProgress);
	QObject::connect(&reader, &TeArchive::Reader::currentFileInfoChanged, [pTracker](const TeArchive::FileInfo & info) {pTracker->setText(info.path); });

	QMetaObject::invokeMethod(&reader, "extractAll", Qt::QueuedConnection, Q_ARG(QString, "debug/test2"));
	//wait dispatch event.
	QThread::msleep(100);

	thread.quit();
	thread.wait();
	rcvThread.quit();
	rcvThread.wait();

	EXPECT_LE(100, tracker.maxvalue);
	EXPECT_LE(list.size(), tracker.count);
	EXPECT_GE(tracker.maxvalue, tracker.progress);
	EXPECT_EQ(list.size(), tracker.strList.size());
	if (list.size() == tracker.strList.size()) {
		for (int i = 0; i < tracker.strList.size(); i++) {
			EXPECT_EQ(list[i].toStdString(), tracker.strList[i].toStdString());
		}
	}

	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}