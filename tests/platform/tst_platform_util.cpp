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

// Linux-only: exercises the Qt-based recursive file operations in
// src/platform/linux/platform_util.cpp through the public copyFiles()/
// moveFiles() API (its internal buildDestinations() helper has no seam to
// unit test directly without widening scope).
#include <QtGlobal>
#ifdef Q_OS_LINUX

#include <gmock/gmock.h>
#include "platform/platform_util.h"
#include "platform/TeFileOpProgress.h"
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <sys/stat.h>

using namespace ::testing;

namespace {
	bool makeDirectoryLink(const QString& target, const QString& linkPath)
	{
		return QFile::link(target, linkPath);
	}

	QString makeBlockDevice(QTemporaryDir* root, const QString& blockDevice, const QString& subsystem)
	{
		const QString sysfs = root->filePath("sys");
		const QString device = sysfs + "/devices/" + blockDevice + "/block/" + blockDevice;
		if (!QDir().mkpath(device) || !QDir().mkpath(sysfs + "/class/block") || !QDir().mkpath(sysfs + "/bus/" + subsystem)) {
			return QString();
		}
		if (!makeDirectoryLink(sysfs + "/bus/" + subsystem, sysfs + "/devices/" + blockDevice + "/subsystem")) {
			return QString();
		}
		if (!makeDirectoryLink(device, sysfs + "/class/block/" + blockDevice)) {
			return QString();
		}
		return sysfs;
	}
}

TEST(tst_platform_util, driveActionLabel_continuesAfterZ)
{
	EXPECT_EQ(platform_util_test::driveActionLabel(0), "C");
	EXPECT_EQ(platform_util_test::driveActionLabel(23), "Z");
	EXPECT_EQ(platform_util_test::driveActionLabel(24), "AA");
}

TEST(tst_platform_util, isExternalBlockDevice_acceptsUsbAndThunderboltAncestors)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());
	const QString sysfs = makeBlockDevice(&root, "sdb", "usb");
	ASSERT_FALSE(sysfs.isEmpty());
	EXPECT_TRUE(platform_util_test::isExternalBlockDevice("sdb", sysfs));

	QTemporaryDir thunderboltRoot;
	ASSERT_TRUE(thunderboltRoot.isValid());
	const QString thunderboltSysfs = makeBlockDevice(&thunderboltRoot, "nvme0n1", "thunderbolt");
	ASSERT_FALSE(thunderboltSysfs.isEmpty());
	EXPECT_TRUE(platform_util_test::isExternalBlockDevice("nvme0n1", thunderboltSysfs));
}

TEST(tst_platform_util, isExternalBlockDevice_resolvesMappedDeviceSlaves)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());
	const QString sysfs = makeBlockDevice(&root, "sdb", "usb");
	ASSERT_FALSE(sysfs.isEmpty());
	const QString mapped = sysfs + "/devices/virtual/block/dm-0";
	ASSERT_TRUE(QDir().mkpath(mapped + "/slaves"));
	ASSERT_TRUE(makeDirectoryLink(mapped, sysfs + "/class/block/dm-0"));
	ASSERT_TRUE(makeDirectoryLink(sysfs + "/class/block/sdb", mapped + "/slaves/sdb"));
	EXPECT_TRUE(platform_util_test::isExternalBlockDevice("dm-0", sysfs));
}

TEST(tst_platform_util, isExternalBlockDevice_rejectsNonExternalDevices)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());
	const QString sysfs = makeBlockDevice(&root, "nvme0n1", "pci");
	ASSERT_FALSE(sysfs.isEmpty());
	EXPECT_FALSE(platform_util_test::isExternalBlockDevice("nvme0n1", sysfs));
}

TEST(tst_platform_util, copyFiles_duplicateSources_rejectedWithoutPartialCopy)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile f(srcFile);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	// Same source path listed twice must be rejected up-front, not partially
	// copied then fail on the second (now-conflicting) destination.
	const bool result = copyFiles({ srcFile, srcFile }, destDir.path());

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

TEST(tst_platform_util, copyFiles_nonexistentSource_rejectedWithoutPartialCopy)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString validSrc = srcDir.filePath("a.txt");
	QFile f(validSrc);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	const QString missingSrc = srcDir.filePath("does-not-exist.txt");

	// A missing source anywhere in the list must fail validation before any
	// entry is copied, even though "a.txt" alone would be valid.
	const bool result = copyFiles({ validSrc, missingSrc }, destDir.path());

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

TEST(tst_platform_util, copyFiles_validDistinctSources_succeeds)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile f(srcFile);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	EXPECT_TRUE(copyFiles({ srcFile }, destDir.path()));
	EXPECT_TRUE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

TEST(tst_platform_util, moveFiles_duplicateSources_rejectedWithoutPartialMove)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile f(srcFile);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	const bool result = moveFiles({ srcFile, srcFile }, destDir.path());

	EXPECT_FALSE(result);
	// Rejected before mutation: the source must still exist and nothing
	// must have appeared at the destination.
	EXPECT_TRUE(QFileInfo(srcFile).exists());
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

TEST(tst_platform_util, copyFiles_destinationEqualsSourceDirectory_rejected)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());

	const QString sourceDir = root.filePath("source");
	ASSERT_TRUE(QDir().mkpath(sourceDir));
	QFile f(QDir(sourceDir).filePath("a.txt"));
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("data");
	f.close();

	// Copying a directory into itself must be rejected before any mutation,
	// not just fail because "source" already exists at the destination.
	const bool result = copyFiles({ sourceDir }, sourceDir);

	EXPECT_FALSE(result);
}

TEST(tst_platform_util, copyFiles_destinationNestedInsideSource_rejectedWithoutPartialCopy)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());

	const QString sourceDir = root.filePath("source");
	ASSERT_TRUE(QDir().mkpath(sourceDir));
	QFile f(QDir(sourceDir).filePath("a.txt"));
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("data");
	f.close();

	// Destination directory is itself nested inside the source directory:
	// copying "source" there would make copyEntryRecursive() recurse into its
	// own freshly-created output.
	const QString nestedDest = QDir(sourceDir).filePath("nested/inside");
	ASSERT_TRUE(QDir().mkpath(nestedDest));

	const bool result = copyFiles({ sourceDir }, nestedDest);

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(QDir(nestedDest).filePath("source")).exists());
}

TEST(tst_platform_util, moveFiles_destinationNestedInsideSource_rejectedWithoutPartialMove)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());

	const QString sourceDir = root.filePath("source");
	ASSERT_TRUE(QDir().mkpath(sourceDir));
	QFile f(QDir(sourceDir).filePath("a.txt"));
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("data");
	f.close();

	const QString nestedDest = QDir(sourceDir).filePath("nested/inside");
	ASSERT_TRUE(QDir().mkpath(nestedDest));

	const bool result = moveFiles({ sourceDir }, nestedDest);

	EXPECT_FALSE(result);
	EXPECT_TRUE(QFileInfo(sourceDir).exists());
	EXPECT_FALSE(QFileInfo(QDir(nestedDest).filePath("source")).exists());
}

TEST(tst_platform_util, copyFile_destinationNestedInsideSourceDirectory_rejected)
{
	QTemporaryDir root;
	ASSERT_TRUE(root.isValid());

	const QString sourceDir = root.filePath("source");
	ASSERT_TRUE(QDir().mkpath(sourceDir));
	QFile f(QDir(sourceDir).filePath("a.txt"));
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("data");
	f.close();

	const QString nestedDest = QDir(sourceDir).filePath("sub/renamed");
	ASSERT_TRUE(QDir().mkpath(QDir(sourceDir).filePath("sub")));

	const bool result = copyFile(sourceDir, nestedDest);

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(nestedDest).exists());
}

TEST(tst_platform_util, copyFile_existingDestination_rejectedWithoutOverwrite)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile src(srcFile);
	ASSERT_TRUE(src.open(QIODevice::WriteOnly));
	src.write("new content");
	src.close();

	const QString destFile = destDir.filePath("a.txt");
	QFile dest(destFile);
	ASSERT_TRUE(dest.open(QIODevice::WriteOnly));
	dest.write("original");
	dest.close();

	const bool result = copyFile(srcFile, destFile);

	EXPECT_FALSE(result);
	QFile check(destFile);
	ASSERT_TRUE(check.open(QIODevice::ReadOnly));
	EXPECT_EQ(check.readAll(), QByteArray("original"));
}

TEST(tst_platform_util, copyFiles_symlinkEntry_preservedAndZeroByteProgress)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString containerDir = srcDir.filePath("container");
	ASSERT_TRUE(QDir().mkpath(containerDir));

	const QString targetFile = QDir(containerDir).filePath("target.txt");
	QFile target(targetFile);
	ASSERT_TRUE(target.open(QIODevice::WriteOnly));
	target.write("hello world");
	target.close();

	const QString linkFile = QDir(containerDir).filePath("link.txt");
	ASSERT_TRUE(QFile::link(targetFile, linkFile));

	TeFileOpProgress progress;
	const bool result = copyFiles({ containerDir }, destDir.path(), 0, &progress);

	ASSERT_TRUE(result);
	// The symlink itself must be recreated as a link, not followed/expanded
	// into a copy of its target's content.
	const QString copiedLink = QDir(destDir.path()).filePath("container/link.txt");
	EXPECT_TRUE(QFileInfo(copiedLink).isSymLink());

	// Symlinks contribute 0 bytes to both total and processed, so once the
	// operation completes the two must match exactly (no perpetual "not quite
	// 100%" progress bar).
	const TeFileOpProgress::Snapshot snap = progress.snapshot();
	EXPECT_EQ(snap.processedBytes, snap.totalBytes);
	EXPECT_GT(snap.totalBytes, 0);
}

TEST(tst_platform_util, copyFile_specialFileSource_rejected)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString fifoPath = srcDir.filePath("pipe");
	ASSERT_EQ(::mkfifo(QFile::encodeName(fifoPath).constData(), 0600), 0);

	const QString destPath = QDir(destDir.path()).filePath("pipe");
	const bool result = copyFile(fifoPath, destPath);

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(destPath).exists());
}

TEST(tst_platform_util, copyFiles_specialFileNested_rejectsAndCleansUpOnlyItsOwnPartialDestination)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	// Pre-existing, unrelated destination entry that must survive the
	// rollback of the failed operation below.
	const QString preexisting = QDir(destDir.path()).filePath("keep-me.txt");
	QFile keep(preexisting);
	ASSERT_TRUE(keep.open(QIODevice::WriteOnly));
	keep.write("keep");
	keep.close();

	const QString containerDir = srcDir.filePath("container");
	ASSERT_TRUE(QDir().mkpath(containerDir));
	QFile regular(QDir(containerDir).filePath("a.txt"));
	ASSERT_TRUE(regular.open(QIODevice::WriteOnly));
	regular.write("data");
	regular.close();

	const QString fifoPath = QDir(containerDir).filePath("a.fifo");
	ASSERT_EQ(::mkfifo(QFile::encodeName(fifoPath).constData(), 0600), 0);

	const bool result = copyFiles({ containerDir }, destDir.path());

	EXPECT_FALSE(result);
	// The newly-created top-level destination entry for this operation is
	// fully rolled back (partial copy cleanup)...
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("container")).exists());
	// ...but a pre-existing, unrelated destination entry is left untouched.
	EXPECT_TRUE(QFileInfo(preexisting).exists());
}

TEST(tst_platform_util, copyFiles_preCancelled_abortsDuringInitialScanWithoutMutation)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile f(srcFile);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	// Cancelling before the call must be observed by the (otherwise
	// blocking) initial total-byte scan itself, not just by the per-entry
	// loop afterwards, so a large pre-cancelled scan cannot stall shutdown.
	TeFileOpProgress progress;
	progress.requestCancel();

	const bool result = copyFiles({ srcFile }, destDir.path(), 0, &progress);

	EXPECT_FALSE(result);
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

TEST(tst_platform_util, moveFiles_preCancelled_abortsDuringInitialScanWithoutMutation)
{
	QTemporaryDir srcDir;
	QTemporaryDir destDir;
	ASSERT_TRUE(srcDir.isValid());
	ASSERT_TRUE(destDir.isValid());

	const QString srcFile = srcDir.filePath("a.txt");
	QFile f(srcFile);
	ASSERT_TRUE(f.open(QIODevice::WriteOnly));
	f.write("hello");
	f.close();

	TeFileOpProgress progress;
	progress.requestCancel();

	const bool result = moveFiles({ srcFile }, destDir.path(), 0, &progress);

	EXPECT_FALSE(result);
	// The source must survive untouched and nothing must appear at the
	// destination: cancellation observed during the initial scan must abort
	// before any rename/copy is attempted.
	EXPECT_TRUE(QFileInfo(srcFile).exists());
	EXPECT_FALSE(QFileInfo(QDir(destDir.path()).filePath("a.txt")).exists());
}

#endif // Q_OS_LINUX



