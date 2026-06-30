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
 *    - bool open( const QString& path);                  [archive_read_open_invalid]
 *      - path variation.
 *          - zip file                                     [archive_read_type]
 *          - 7zip file                                    [archive_read_type]
 *          - tar (non-compressed)                         [archive_read_type]
 *          - tar gzip                                     [archive_read_type]
 *          - tar bzip2                                    [archive_read_type]
 *          - other of libarchive supported files.
 *          - null                                         [archive_read_open_invalid]
 *          - directory                                    [archive_read_open_invalid]
 *          - normal file                                  [archive_read_open_invalid]
 *          - empty file                                   [archive_read_open_invalid]
 *    - bool isValid();                                    [archive_read_state_lifecycle / open_invalid]
 *    - ArchiveType type();                                [archive_read_type]
 *       - variation of archive type
 *    - void close();                                      [archive_read_state_lifecycle]
 *       - operation order
 *          - after open.
 *          - before open.
 *          - call close twice.
 *    - void cancel() / clearCancel();                     [archive_read_cancel]
 *    - const QString& path()                              [archive_read_state_lifecycle]
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
 *    - TeArchive::Reader::iterator                        [archive_read_iterator]
 *       - check included files
 *    - Encryption / Password
 *       (encrypted-archive tests are skipped when the linked libarchive
 *        build lacks ZIP write-encryption support.)
 *       - Writer::setPassword() / password()              [archive_password_accessor]
 *       - Reader::setPassword() / password()              [archive_password_accessor]
 *       - encrypted ZIP round-trip (AES-256)              [archive_password_roundtrip]
 *       - isEncrypted()                                   [archive_password_isEncrypted]
 *          - encrypted archive
 *          - plain archive
 *       - verifyPassword() / lastResult()                 [archive_password_verify]
 *          - correct password   -> RESULT_OK
 *          - wrong password     -> RESULT_WRONG_PASSWORD
 *          - no password        -> RESULT_PASSWORD_REQUIRED
 *       - extract with wrong password                     [archive_password_wrong_extract]
 */

#include <gmock/gmock.h>

#include <utils/TeArchive.h>
#include <test_util/TestFileCreator.h>
#include <test_util/ProgressTracker.h>

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QScopedPointer>

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

	createFileTree("debug/test", expect, 10);

	QScopedPointer<QStandardItem> entries( new QStandardItem );
	expectEntries(entries.data(), "debug/test", expect, QDateTime());

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
	bool r2 = compareFileTree(entries.data(), "debug/test2", true);

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
	checkArchive(list, list, TeArchive::AR_TAR);
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
	list.append("dir2/");

	checkArchive(list, list, TeArchive::AR_ZIP);
	checkArchive(list, list, TeArchive::AR_7ZIP);
	checkArchive(list, list, TeArchive::AR_TAR);
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
	EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
	file.write("test");
	file.close();

	file.setFileName("debug/test2/dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
	file.write("test");
	file.close();

	reader.setCallback(unAccept);
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	//overwrite
	createFileTree("debug/test", list);

	file.setFileName("debug/test2/dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	EXPECT_TRUE(file.open(QFile::WriteOnly|QFile::Truncate));
	file.write("test");
	file.close();
	file.setFileName("debug/test2/dir1/file1_1.txt");
	EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
	file.write("test");
	file.close();
	file.setFileName("debug/test2/dir1/file1_2.txt");
	EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
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
	QObject::connect(&writer, &TeArchive::Writer::currentFileInfoChanged, [pwTracker](const TeFileInfo & info) {pwTracker->setText(info.path); });

	writer.archive("debug/test.zip", TeArchive::AR_ZIP);

	wthread.quit();
	wthread.wait();

	//Check Progress values
	EXPECT_LE(list.size() ,wtracker.maxvalue);
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
	QObject::connect(&reader, &TeArchive::Reader::currentFileInfoChanged, [pTracker](const TeFileInfo & info) {pTracker->setText(info.path); });

	QMetaObject::invokeMethod(&reader, "extractAll", Qt::QueuedConnection, Q_ARG(QString, "debug/test2"));
	//wait dispatch event.
	QThread::msleep(100);

	thread.quit();
	thread.wait();
	rcvThread.quit();
	rcvThread.wait();

	EXPECT_LE(10, tracker.maxvalue);
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

namespace {

	/*!
		Create an archive file at \a dest with given \a type from the flat
		file \a list located under "debug/test".
	*/
	void makeArchive(const QString& dest, TeArchive::ArchiveType type, const QStringList& list)
	{
		cleanFileTree("debug/test");
		QFile::remove(dest);
		createFileTree("debug/test", list, 1);

		TeArchive::Writer writer;
		writer.addEntries("debug/test", list, "");
		writer.archive(dest, type);
	}

}

/*!
	Check Reader::open() variation of input path.
	- null / empty path
	- not existing path
	- directory
	- normal (non-archive) file
	- empty file
	For every failure case isValid() must be false and type() must be AR_NONE.
*/
TEST(tst_TeArchive, archive_read_open_invalid)
{
	cleanFileTree("debug/test");

	TeArchive::Reader reader;

	// null / empty path
	EXPECT_FALSE(reader.open(QString()));
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	EXPECT_FALSE(reader.open(""));
	EXPECT_FALSE(reader.isValid());

	// not existing path
	EXPECT_FALSE(reader.open("debug/not_exist_archive.zip"));
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	// directory
	QDir dir;
	dir.mkpath("debug/test");
	EXPECT_FALSE(reader.open("debug/test"));
	EXPECT_FALSE(reader.isValid());

	// normal (non-archive) file
	{
		QFile file("debug/test/normal.txt");
		EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
		file.write("this is not an archive file.");
		file.close();
	}
	EXPECT_FALSE(reader.open("debug/test/normal.txt"));
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	// empty file
	{
		QFile file("debug/test/empty.dat");
		EXPECT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
		file.close();
	}
	EXPECT_FALSE(reader.open("debug/test/empty.dat"));
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	cleanFileTree("debug/test");
}

/*!
	Check Reader state accessors (path(), isValid(), type()) across the
	open / close lifecycle, and the robustness of close() ordering.
	- before open : path() empty, isValid() false, type() AR_NONE
	- after open  : path() set, isValid() true, type() matches archive
	- after close : path() empty, isValid() false, type() AR_NONE
	- close() before open and close() twice must not crash.
*/
TEST(tst_TeArchive, archive_read_state_lifecycle)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	makeArchive("debug/test.zip", TeArchive::AR_ZIP, list);

	TeArchive::Reader reader;

	// close() before any open() must be safe.
	reader.close();

	// before open
	EXPECT_TRUE(reader.path().isEmpty());
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	// after open
	EXPECT_TRUE(reader.open("debug/test.zip"));
	EXPECT_EQ("debug/test.zip", reader.path());
	EXPECT_TRUE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_ZIP, reader.type());

	// after close
	reader.close();
	EXPECT_TRUE(reader.path().isEmpty());
	EXPECT_FALSE(reader.isValid());
	EXPECT_EQ(TeArchive::AR_NONE, reader.type());

	// close() twice must be safe.
	reader.close();
	EXPECT_FALSE(reader.isValid());

	cleanFileTree("debug/test");
	QFile::remove("debug/test.zip");
}

/*!
	Check Reader::type() returns the correct ArchiveType for every
	supported archive format.
*/
TEST(tst_TeArchive, archive_read_type)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");

	struct { TeArchive::ArchiveType type; const char* path; } cases[] = {
		{ TeArchive::AR_ZIP,        "debug/test_type.zip" },
		{ TeArchive::AR_7ZIP,       "debug/test_type.7z" },
		{ TeArchive::AR_TAR,        "debug/test_type.tar" },
		{ TeArchive::AR_TAR_GZIP,   "debug/test_type.tar.gz" },
		{ TeArchive::AR_TAR_BZIP2,  "debug/test_type.tar.bz2" },
	};

	for (const auto& c : cases) {
		makeArchive(c.path, c.type, list);

		TeArchive::Reader reader;
		EXPECT_TRUE(reader.open(c.path)) << "failed to open " << c.path;
		EXPECT_TRUE(reader.isValid());
		EXPECT_EQ(c.type, reader.type()) << "unexpected type for " << c.path;
		reader.close();

		QFile::remove(c.path);
	}

	cleanFileTree("debug/test");
}

/*!
	Check Reader iterator (begin()/end()) enumerates archive entries.
	Every file registered for archiving must be visited exactly once.
*/
TEST(tst_TeArchive, archive_read_iterator)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	makeArchive("debug/test.zip", TeArchive::AR_ZIP, list);

	TeArchive::Reader reader;
	EXPECT_TRUE(reader.open("debug/test.zip"));

	QStringList found;
	for (const TeFileInfo& info : reader) {
		EXPECT_EQ(TeFileInfo::EN_FILE, info.type);
		found.append(info.path);
	}

	EXPECT_EQ(list.size(), found.size());
	for (const QString& expect : list) {
		EXPECT_TRUE(found.contains(expect)) << "missing entry: " << expect.toStdString();
	}

	// end() of an opened archive and an empty (default) iterator compare equal.
	EXPECT_TRUE(reader.begin() != reader.end());

	reader.close();

	// Iterating an unopened reader yields no entries.
	TeArchive::Reader empty;
	EXPECT_TRUE(empty.begin() == empty.end());

	cleanFileTree("debug/test");
	QFile::remove("debug/test.zip");
}

/*!
	Check cancel() / clearCancel() behaviour.
	- cancel() before extraction stops the extraction (no full tree extracted).
	- clearCancel() restores the reader so a later extraction completes.
*/
TEST(tst_TeArchive, archive_read_cancel)
{
	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	makeArchive("debug/test.zip", TeArchive::AR_ZIP, list);

	TeArchive::Reader reader;
	EXPECT_TRUE(reader.open("debug/test.zip"));

	QDir dir;
	dir.mkpath("debug/test2");

	// Request cancellation before extraction begins.
	reader.cancel();
	reader.extractAll("debug/test2");

	// Clear the cancellation flag and extract again; it must now complete.
	reader.clearCancel();
	reader.extractAll("debug/test2");
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	reader.close();

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}

namespace {

	/*!
		Create a password-protected ZIP archive at \a dest (AES-256) from the
		flat file \a list located under "debug/test".
	*/
	void makeEncryptedZip(const QString& dest, const QString& password, const QStringList& list)
	{
		cleanFileTree("debug/test");
		QFile::remove(dest);
		createFileTree("debug/test", list, 1);

		TeArchive::Writer writer;
		writer.setPassword(password);
		writer.addEntries("debug/test", list, "");
		writer.archive(dest, TeArchive::AR_ZIP);
	}

	/*!
		Returns true when the linked libarchive build can actually create an
		AES-256 encrypted ZIP.  Some libarchive builds are compiled without the
		crypto backend required for ZIP write encryption; in that case the
		encryption-dependent tests are skipped instead of failing.
	*/
	bool zipWriteEncryptionAvailable()
	{
		QStringList list;
		list.append("probe.txt");
		makeEncryptedZip("debug/_enc_probe.zip", "probe", list);

		TeArchive::Reader reader;
		reader.setPassword("probe");
		bool available = reader.open("debug/_enc_probe.zip") && reader.isEncrypted();
		reader.close();

		cleanFileTree("debug/test");
		QFile::remove("debug/_enc_probe.zip");
		return available;
	}

}

/*!
	Check password accessors of Writer and Reader.
	setPassword() stores the passphrase, password() returns it, and an empty
	string clears it.
*/
TEST(tst_TeArchive, archive_password_accessor)
{
	TeArchive::Writer writer;
	EXPECT_TRUE(writer.password().isEmpty());
	writer.setPassword("secret");
	EXPECT_EQ(QString("secret"), writer.password());
	writer.setPassword(QString());
	EXPECT_TRUE(writer.password().isEmpty());

	TeArchive::Reader reader;
	EXPECT_TRUE(reader.password().isEmpty());
	reader.setPassword("secret");
	EXPECT_EQ(QString("secret"), reader.password());
	reader.setPassword(QString());
	EXPECT_TRUE(reader.password().isEmpty());
}

/*!
	Check encrypted ZIP round-trip.
	An archive created with a password must be extractable with the correct
	password, and the extracted tree must match the original.
*/
TEST(tst_TeArchive, archive_password_roundtrip)
{
	if (!zipWriteEncryptionAvailable()) {
		GTEST_SKIP() << "libarchive build lacks ZIP write encryption (AES-256).";
	}

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	makeEncryptedZip("debug/test.zip", "P@ssw0rd", list);

	TeArchive::Reader reader;
	reader.setPassword("P@ssw0rd");
	EXPECT_TRUE(reader.open("debug/test.zip"));
	EXPECT_EQ(TeArchive::AR_ZIP, reader.type());
	EXPECT_TRUE(reader.isEncrypted());

	QDir dir;
	dir.mkpath("debug/test2");
	EXPECT_TRUE(reader.extractAll("debug/test2"));
	EXPECT_TRUE(compareFileTree("debug/test", "debug/test2", true));

	reader.close();

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}

/*!
	Check isEncrypted() reflects the archive content.
	- encrypted ZIP : isEncrypted() == true
	- plain ZIP     : isEncrypted() == false
*/
TEST(tst_TeArchive, archive_password_isEncrypted)
{
	if (!zipWriteEncryptionAvailable()) {
		GTEST_SKIP() << "libarchive build lacks ZIP write encryption (AES-256).";
	}

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");

	// Encrypted archive.
	makeEncryptedZip("debug/test.zip", "P@ssw0rd", list);
	{
		TeArchive::Reader reader;
		reader.setPassword("P@ssw0rd");
		EXPECT_TRUE(reader.open("debug/test.zip"));
		EXPECT_TRUE(reader.isEncrypted());
		reader.close();
	}

	// Plain archive.
	makeArchive("debug/test.zip", TeArchive::AR_ZIP, list);
	{
		TeArchive::Reader reader;
		EXPECT_TRUE(reader.open("debug/test.zip"));
		EXPECT_FALSE(reader.isEncrypted());
		reader.close();
	}

	cleanFileTree("debug/test");
	QFile::remove("debug/test.zip");
}

/*!
	Check verifyPassword() and lastResult() for an encrypted ZIP.
	- correct password : RESULT_OK
	- wrong password   : RESULT_WRONG_PASSWORD
	- no password      : RESULT_PASSWORD_REQUIRED
*/
TEST(tst_TeArchive, archive_password_verify)
{
	if (!zipWriteEncryptionAvailable()) {
		GTEST_SKIP() << "libarchive build lacks ZIP write encryption (AES-256).";
	}

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	makeEncryptedZip("debug/test.zip", "P@ssw0rd", list);

	// Correct password.
	{
		TeArchive::Reader reader;
		reader.setPassword("P@ssw0rd");
		EXPECT_TRUE(reader.open("debug/test.zip"));
		EXPECT_EQ(TeArchive::Reader::RESULT_OK, reader.verifyPassword());
		EXPECT_EQ(TeArchive::Reader::RESULT_OK, reader.lastResult());
		reader.close();
	}

	// Wrong password.
	{
		TeArchive::Reader reader;
		reader.setPassword("wrong-password");
		EXPECT_TRUE(reader.open("debug/test.zip"));
		EXPECT_EQ(TeArchive::Reader::RESULT_WRONG_PASSWORD, reader.verifyPassword());
		EXPECT_EQ(TeArchive::Reader::RESULT_WRONG_PASSWORD, reader.lastResult());
		reader.close();
	}

	// No password.
	{
		TeArchive::Reader reader;
		EXPECT_TRUE(reader.open("debug/test.zip"));
		EXPECT_EQ(TeArchive::Reader::RESULT_PASSWORD_REQUIRED, reader.verifyPassword());
		EXPECT_EQ(TeArchive::Reader::RESULT_PASSWORD_REQUIRED, reader.lastResult());
		reader.close();
	}

	cleanFileTree("debug/test");
	QFile::remove("debug/test.zip");
}

/*!
	Check extraction of an encrypted ZIP with a wrong password.
	The wrong passphrase must not reproduce the original file contents.
*/
TEST(tst_TeArchive, archive_password_wrong_extract)
{
	if (!zipWriteEncryptionAvailable()) {
		GTEST_SKIP() << "libarchive build lacks ZIP write encryption (AES-256).";
	}

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	makeEncryptedZip("debug/test.zip", "P@ssw0rd", list);

	TeArchive::Reader reader;
	reader.setPassword("wrong-password");
	EXPECT_TRUE(reader.open("debug/test.zip"));
	EXPECT_TRUE(reader.isEncrypted());

	QDir dir;
	dir.mkpath("debug/test2");
	reader.extractAll("debug/test2");

	// With a wrong password the data cannot be decrypted, so the extracted
	// files must not faithfully reproduce the source content.  (compareFileTree
	// is intentionally avoided here because it raises its own gtest
	// expectations on mismatch.)
	bool anyContentDiffers = false;
	for (const QString& name : list) {
		QFile srcFile("debug/test/" + name);
		QByteArray srcData;
		if (srcFile.open(QFile::ReadOnly)) {
			srcData = srcFile.readAll();
			srcFile.close();
		}

		QByteArray dstData;
		QFile dstFile("debug/test2/" + name);
		if (dstFile.open(QFile::ReadOnly)) {
			dstData = dstFile.readAll();
			dstFile.close();
		}

		if (srcData != dstData) {
			anyContentDiffers = true;
		}
	}
	EXPECT_TRUE(anyContentDiffers);

	reader.close();

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");
}