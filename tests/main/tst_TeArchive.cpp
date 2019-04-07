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
 *          - add entry that already added. (for same type)
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
 *    - void release();
 *       - operation order
 *          - after open.
 *          - before open.
 *          - call release twice.
 *    - const QString& path()
 *       - operation order
 *          - before open
 *          - after open
 *          - after release
 *    - bool extractAll(const QString& destPath);
 *       - operation order
 *          - before open
 *          - after open
 *          - after release
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
	writer.archive("debug/test.zip", type);

	reader.open("debug/test.zip");

	QDir dir;
	dir.mkpath("debug/test2");
	reader.extractAll("debug/test2");
	reader.release();

	bool r1 = compareFileTree("debug/test", "debug/test2", true);
	bool r2 = compareFileTree(&entries, "debug/test2", true);

	cleanFileTree("debug/test");
	cleanFileTree("debug/test2");
	QFile::remove("debug/test.zip");

	return r1 && r2;
}

TEST(tst_TeArchive, archive_flatfile_zip)
{
	QStringList list;
	list.append("file1.txt");
	list.append("file2.txt");
	list.append("file3.txt");
	list.append("file4.txt");
	
	checkArchive(list, list, TeArchive::AR_ZIP);
}

TEST(tst_TeArchive, archive_rankfile_zip)
{
	QStringList list;
	list.append("dir1/dir1_1/dir1_1_1/file1_1_1_1.txt");
	list.append("dir3/dir3_2/file3_2_1.txt");
	list.append("dir1/file1_2.txt");
	list.append("dir1/file1_1.txt");

	checkArchive(list, list, TeArchive::AR_ZIP);
}