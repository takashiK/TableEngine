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

#include "platform/platform_util.h"
#include "platform/TeNativeEvent.h"
#include "platform/TeFileOpProgress.h"
#include "platform/linux/TeLinuxMountMonitor.h"
#include "dialogs/TePropertiesDialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QVector>
#include <QSet>
#include <QStorageInfo>

#include <QMimeDatabase>
#include <QMimeType>
#include <QIcon>
#include <QImageReader>
#include <QStandardPaths>
#include <QCryptographicHash>

#include <algorithm>
#include <sys/stat.h>

/**
 * @file platform_util.cpp
 * @brief Linux (Ubuntu / freedesktop.org) implementation of platform_util.
 * @ingroup platform
 *
 * @details File copy/move/delete are implemented with Qt-only recursive
 * filesystem operations (no shell/D-Bus dependency): symlinks are copied as
 * links rather than followed, hidden entries are included, name conflicts
 * fail the operation rather than overwriting, and permissions/mtimes are
 * preserved where the platform/Qt API supports it. Deletion always goes
 * through QFile::moveToTrash() and never falls back to a permanent delete.
 * Thumbnails are read (never generated) from the freedesktop.org thumbnail
 * cache, file icons come from the MIME/icon-theme lookup, and clipboard
 * cut/copy interoperates with the GNOME and KDE conventions in addition to
 * the Windows-style hint already used for same-app drag/drop. Context menus
 * and "open with" association lookup have no desktop-agnostic equivalent and
 * remain no-op / caller-side fallback. Mount changes are monitored through
 * /proc/self/mountinfo and only refresh the external-drive projection.
 */

namespace {
	TeNativeEvent g_event;
	TeLinuxMountMonitor* g_mountMonitor = nullptr;
	platform_util_test::DriveActionLabelAllocator g_driveActionLabelAllocator;

	//////////////////////////////////////////////////////////////
	//
	// Filesystem helpers shared by copyFiles/copyFile/moveFiles/deleteFiles
	//

	// True if something exists at @p path, including a dangling symlink
	// (QFileInfo::exists() follows symlinks and would say false for those).
	bool pathEntryExists(const QString& path)
	{
		QFileInfo info(path);
		return info.exists() || info.isSymLink();
	}

	// Recursively sums entry sizes without following symlinks into directories.
	// Symlinks contribute 0 bytes (matching copyEntryRecursive(), which reports
	// 0 processed bytes for them since they are recreated, not content-copied),
	// so total/processed stay consistent and progress can reach 100%.
	// When @p progress is non-null, the scan itself observes cancellation and
	// returns early (with a partial/meaningless total) so a large initial scan
	// cannot block TeFileOperationManager's destructor cancellation for long;
	// the caller is expected to re-check progress->isCancelled() right after.
	qint64 computeTotalBytes(const QStringList& paths, TeFileOpProgress* progress = nullptr)
	{
		qint64 total = 0;
		QVector<QFileInfo> stack;
		for (const QString& p : paths) {
			stack.append(QFileInfo(p));
		}
		while (!stack.isEmpty()) {
			if (progress != nullptr && progress->isCancelled()) {
				break;
			}
			const QFileInfo info = stack.takeLast();
			if (info.isSymLink()) {
				continue;
			}
			if (!info.isDir()) {
				total += info.size();
				continue;
			}
			const QFileInfoList children = QDir(info.filePath())
				.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
			for (const QFileInfo& child : children) {
				stack.append(child);
			}
		}
		return total;
	}

	// Computes the same total as computeTotalBytes(paths, progress) but also
	// returns each top-level entry's own size via @p outSizes (indices aligned
	// with @p paths), so callers that need a per-entry size while processing
	// each entry (moveFiles()/deleteFiles() progress reporting) do not have to
	// redundantly re-scan the same subtree a second time.
	qint64 computeEntrySizes(const QStringList& paths, TeFileOpProgress* progress, QVector<qint64>* outSizes)
	{
		qint64 total = 0;
		outSizes->reserve(paths.size());
		for (const QString& p : paths) {
			if (progress != nullptr && progress->isCancelled()) {
				outSizes->append(0);
				continue;
			}
			const qint64 size = computeTotalBytes({ p }, progress);
			outSizes->append(size);
			total += size;
		}
		return total;
	}

	// True if @p sourcePath and @p destDir reside on the same filesystem/device,
	// so a rename(2) between them can be atomic. @p sourcePath uses lstat() (not
	// stat()) so a symlink source is checked by its own device rather than by
	// its target's, matching rename(2)'s own semantics of operating on the link
	// itself. @p destDir uses stat() (following symlinks) because it is always
	// a directory we move into: if it is itself a symlink to another
	// filesystem, comparing its own (symlink) device would wrongly report
	// "same device" and let QFile::rename() slip through to Qt's hidden
	// cross-device copy fallback (see moveFiles()).
	bool sameDevice(const QString& sourcePath, const QString& destDir)
	{
		struct stat statSource {};
		struct stat statDest {};
		if (::lstat(QFile::encodeName(sourcePath).constData(), &statSource) != 0) {
			return false;
		}
		if (::stat(QFile::encodeName(destDir).constData(), &statDest) != 0) {
			return false;
		}
		return statSource.st_dev == statDest.st_dev;
	}

	// True if @p candidate is the same filesystem entry as @p base, or located
	// anywhere below it. Used to reject a copy/move whose destination would end
	// up nested inside the very directory being copied, which would otherwise
	// make copyEntryRecursive() recurse into its own freshly-created output.
	// @p base must already exist; @p candidate does not need to (only its
	// longest existing ancestor is canonicalized, and any remaining not-yet-
	// created path components are reattached unchanged).
	bool isPathWithinOrEqual(const QString& base, const QString& candidate)
	{
		const QString canonicalBase = QFileInfo(base).canonicalFilePath();
		if (canonicalBase.isEmpty()) {
			return false;
		}

		QString existingPart = QDir::cleanPath(candidate);
		QStringList pendingParts;
		while (!existingPart.isEmpty() && !QFileInfo(existingPart).exists()) {
			const QFileInfo info(existingPart);
			pendingParts.prepend(info.fileName());
			const QString parent = info.path();
			if (parent == existingPart) {
				break; // reached root without finding an existing ancestor
			}
			existingPart = parent;
		}
		if (existingPart.isEmpty()) {
			return false;
		}

		QString canonicalCandidate = QFileInfo(existingPart).canonicalFilePath();
		if (canonicalCandidate.isEmpty()) {
			return false;
		}
		if (!pendingParts.isEmpty()) {
			canonicalCandidate += QLatin1Char('/') + pendingParts.join(QLatin1Char('/'));
		}

		return canonicalCandidate == canonicalBase || canonicalCandidate.startsWith(canonicalBase + QLatin1Char('/'));
	}

	// True for filesystem entries whose content cannot be meaningfully copied
	// byte-for-byte (FIFOs, character/block devices, sockets). Symlinks and
	// directories are handled by dedicated branches before this is reached;
	// QFileInfo::isFile()/isDir()/isSymLink() are all false for these entries,
	// so no extra syscall is needed to detect them.
	bool isSpecialFile(const QFileInfo& info)
	{
		return pathEntryExists(info.filePath()) && !info.isFile() && !info.isDir() && !info.isSymLink();
	}

	// Copies file contents in chunks so progress can be reported and
	// cancellation can take effect mid-file, instead of blocking for the
	// whole file inside a single QFile::copy() call.
	bool copyFileContents(const QString& srcPath, const QString& destPath, TeFileOpProgress* progress)
	{
		QFile in(srcPath);
		if (!in.open(QIODevice::ReadOnly)) {
			return false;
		}
		QFile out(destPath);
		if (!out.open(QIODevice::WriteOnly)) {
			return false;
		}

		constexpr qint64 kChunkSize = 4 * 1024 * 1024;
		QByteArray buffer(kChunkSize, Qt::Uninitialized);
		// Loop on read()'s own return value rather than QFile::atEnd(): atEnd()
		// can report true prematurely for zero-byte pseudo-files (e.g. procfs/
		// sysfs entries whose stat() size is 0 despite having real content), which
		// would otherwise silently produce an empty destination that moveFiles()
		// would then treat as a successful copy before deleting the real source.
		for (;;) {
			if (progress != nullptr && progress->isCancelled()) {
				out.close();
				QFile::remove(destPath);
				return false;
			}
			const qint64 bytesRead = in.read(buffer.data(), kChunkSize);
			if (bytesRead < 0) {
				out.close();
				QFile::remove(destPath);
				return false;
			}
			if (bytesRead == 0) {
				break; // EOF, per read()'s own signal rather than atEnd()
			}
			if (out.write(buffer.constData(), bytesRead) != bytesRead) {
				out.close();
				QFile::remove(destPath);
				return false;
			}
			if (progress != nullptr) {
				progress->addProcessed(bytesRead, srcPath);
			}
		}

		// QFileDevice::close() is void, so a failed final flush (e.g. ENOSPC, or
		// an NFS write error that only surfaces at close()) would otherwise go
		// unnoticed and this function would report success over a truncated/
		// corrupt destination -- which moveFiles() would then trust enough to
		// delete the still-intact source. flush()'s bool result is captured
		// before close() (which internally calls flush() again for buffered
		// writes but discards its result), and out.error() is re-checked after
		// close() too, in case the close() syscall itself reports a late error.
		const bool flushed = out.flush();
		out.close();
		if (!flushed || out.error() != QFile::NoError) {
			QFile::remove(destPath);
			return false;
		}

		// Preserve mtime/permissions "where supported"; failures here are not
		// fatal to the copy itself.
		QFile destFile(destPath);
		if (destFile.open(QIODevice::ReadWrite)) {
			destFile.setFileTime(QFileInfo(srcPath).lastModified(), QFileDevice::FileModificationTime);
			destFile.close();
		}
		QFile::setPermissions(destPath, QFileInfo(srcPath).permissions());
		return true;
	}

	// Outcome of copyEntryRecursive(), distinguishing "this call created
	// nothing" from "this call created (part of) destPath but did not finish
	// it", so callers know whether it is safe to roll back destPath: a
	// Conflict at the top level means another process created that entry
	// between buildDestinations() validating it was free and this call
	// running, and that entry must never be deleted by this operation's
	// rollback.
	enum class CopyEntryStatus {
		Ok,
		Conflict,
		Failed
	};

	// Recursively copies one entry (file, directory, or symlink) to destPath.
	// Symlinks are recreated pointing at the same target rather than followed;
	// an existing entry at destPath (including a dangling symlink) fails the
	// copy rather than being overwritten.
	CopyEntryStatus copyEntryRecursive(const QFileInfo& srcInfo, const QString& destPath, TeFileOpProgress* progress)
	{
		if (progress != nullptr && progress->isCancelled()) {
			return CopyEntryStatus::Failed;
		}
		if (pathEntryExists(destPath)) {
			return CopyEntryStatus::Conflict; // conflict: never overwrite, nothing created here
		}

		if (srcInfo.isSymLink()) {
			// Qt resolves the link target to an absolute path, so a relative
			// symlink becomes an absolute one after copying; it still resolves
			// to the same file.
			const QString target = srcInfo.symLinkTarget();
			if (target.isEmpty()) {
				return CopyEntryStatus::Failed; // an empty target is not a meaningful link to recreate
			}
			if (!QFile::link(target, destPath)) {
				return CopyEntryStatus::Failed;
			}
			if (progress != nullptr) {
				progress->addProcessed(0, srcInfo.filePath());
			}
			return CopyEntryStatus::Ok;
		}

		if (srcInfo.isDir()) {
			if (!QDir().mkpath(destPath)) {
				return CopyEntryStatus::Failed;
			}
			const QFileInfoList children = QDir(srcInfo.filePath())
				.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
			for (const QFileInfo& child : children) {
				// destPath (this directory) was created by this call regardless
				// of whether the failing child hit Conflict or Failed, so any
				// non-Ok child result makes this call's own result Failed.
				if (copyEntryRecursive(child, destPath + QLatin1Char('/') + child.fileName(), progress) != CopyEntryStatus::Ok) {
					return CopyEntryStatus::Failed;
				}
			}
			QFile::setPermissions(destPath, srcInfo.permissions());
			return CopyEntryStatus::Ok;
		}

		if (isSpecialFile(srcInfo)) {
			return CopyEntryStatus::Failed; // FIFOs/sockets/devices cannot be content-copied safely
		}

		return copyFileContents(srcInfo.filePath(), destPath, progress) ? CopyEntryStatus::Ok : CopyEntryStatus::Failed;
	}

	// Permanently removes an entry (used only to finish a move whose content
	// has already been safely copied to the destination).
	bool removeEntryRecursive(const QFileInfo& info)
	{
		if (info.isSymLink() || !info.isDir()) {
			return QFile::remove(info.filePath());
		}
		const QFileInfoList children = QDir(info.filePath())
			.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
		for (const QFileInfo& child : children) {
			if (!removeEntryRecursive(child)) {
				return false;
			}
		}
		return QDir().rmdir(info.filePath());
	}

	// Builds/validates the per-source destination path list for copy/move
	// into a target directory; returns false (without side effects) if the
	// input is empty/invalid, any source is missing, or any destination
	// already exists or is duplicated by another source in this same call
	// (duplicate sources would otherwise map to one destination and cause a
	// partial operation once the first copy creates it).
	bool buildDestinations(const QStringList& sources, const QString& destDir, QStringList* destinations)
	{
		if (sources.isEmpty() || destDir.isEmpty()) {
			return false;
		}
		if (!QFileInfo(destDir).isDir()) {
			return false;
		}
		QStringList candidates;
		QSet<QString> seenDestinations;
		for (const QString& src : sources) {
			if (src.isEmpty() || !pathEntryExists(src)) {
				return false; // invalid/nonexistent source
			}
			const QFileInfo srcInfo(src);
			if (!srcInfo.isSymLink() && srcInfo.isDir() && isPathWithinOrEqual(src, destDir)) {
				return false; // destDir is src itself, or nested below it: would self-recurse
			}
			const QString dest = QDir(destDir).filePath(srcInfo.fileName());
			if (pathEntryExists(dest) || seenDestinations.contains(dest)) {
				return false; // conflict: existing entry or duplicate source
			}
			seenDestinations.insert(dest);
			candidates.append(dest);
		}
		*destinations = candidates;
		return true;
	}

	bool isExternalBlockDeviceImpl(const QString& blockDevice, const QString& sysfsRoot, QSet<QString>* visited)
	{
		if (blockDevice.isEmpty() || visited->contains(blockDevice)) {
			return false;
		}
		visited->insert(blockDevice);

		const QString root = QDir::cleanPath(sysfsRoot);
		const QString entry = QFileInfo(QDir(root).filePath(QStringLiteral("class/block/") + blockDevice)).canonicalFilePath();
		if (entry.isEmpty() || (entry != root && !entry.startsWith(root + QLatin1Char('/')))) {
			return false;
		}

		QString current = entry;
		while (current == root || current.startsWith(root + QLatin1Char('/'))) {
			const QString subsystem = QFileInfo(QDir(current).filePath(QStringLiteral("subsystem"))).canonicalFilePath();
			const QString subsystemName = QFileInfo(subsystem).fileName();
			if (subsystemName == QStringLiteral("usb") || subsystemName == QStringLiteral("thunderbolt")) {
				return true;
			}
			if (current == root) {
				break;
			}
			current = QFileInfo(current).dir().absolutePath();
		}

		const QDir slaves(QDir(entry).filePath(QStringLiteral("slaves")));
		for (const QFileInfo& slave : slaves.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			if (isExternalBlockDeviceImpl(slave.fileName(), root, visited)) {
				return true;
			}
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////
//
// Initialize and uninitialize platform system
//
bool threadInitialize(QApplication* a)
{
	if (a != nullptr && g_mountMonitor == nullptr) {
		g_mountMonitor = new TeLinuxMountMonitor(&g_event, a);
		QObject::connect(a, &QObject::destroyed, [] { g_mountMonitor = nullptr; });
		g_mountMonitor->start();
	}
	return true;
}

void threadUninitialize()
{
	delete g_mountMonitor;
	g_mountMonitor = nullptr;
}

void comInitializeThread()
{
}

void comUninitializeThread()
{
}

QList<TeDriveAction> getDriveActions()
{
	QList<TeDriveAction> actions;
	for (const QStorageInfo& drive : QStorageInfo::mountedVolumes()) {
		const QString devicePath = QFileInfo(drive.device()).canonicalFilePath();
		const QString blockDevice = QFileInfo(devicePath.isEmpty() ? drive.device() : devicePath).fileName();
		if (!platform_util_test::isExternalBlockDevice(blockDevice, QStringLiteral("/sys"))) {
			continue;
		}

		const QString mountPoint = drive.rootPath();
		QString mountName = QFileInfo(QDir::cleanPath(mountPoint)).fileName();
		if (mountName.isEmpty()) {
			mountName = mountPoint;
		}
		QString toolTip = drive.displayName();
		if (toolTip.isEmpty()) {
			toolTip = mountPoint;
		}
		const QString driveIdentity = (devicePath.isEmpty() ? drive.device() : devicePath)
			+ QChar::Null + QDir::cleanPath(mountPoint);
		actions.append({ mountName, mountPoint, toolTip, driveIdentity });
	}
	g_driveActionLabelAllocator.assign(&actions);
	return actions;
}

//////////////////////////////////////////////////////////////
//
// File action
//
bool showFilesContext(int /*px*/, int /*py*/, const QStringList& /*paths*/)
{
	// No desktop-agnostic native context menu; callers fall back to the
	// user-configurable popup menu when this returns false.
	return false;
}

void showFileContext(int /*px*/, int /*py*/, const QString& /*path*/)
{
}

void showFilesProperties(const QStringList& paths)
{
	if (paths.isEmpty()) {
		return;
	}
	// No native "properties" dialog on Linux; reuse the app's own dialog.
	TePropertiesDialog dlg(TePropertiesDialog::fromPaths(paths), true);
	dlg.exec();
}

void showFileProperties(const QString& path)
{
	if (path.isEmpty()) {
		return;
	}
	showFilesProperties(QStringList{ path });
}

void openFile(const QString& path)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool copyFiles(const QStringList& files, const QString& path, WId /*owner*/, TeFileOpProgress* progress)
{
	QStringList destinations;
	if (!buildDestinations(files, path, &destinations)) {
		return false;
	}

	if (progress != nullptr) {
		progress->start(computeTotalBytes(files, progress));
		if (progress->isCancelled()) {
			return false;
		}
	}

	for (int i = 0; i < files.size(); ++i) {
		if (progress != nullptr && progress->isCancelled()) {
			return false;
		}
		const CopyEntryStatus status = copyEntryRecursive(QFileInfo(files[i]), destinations[i], progress);
		if (status == CopyEntryStatus::Failed) {
			// Only the in-flight entry may have partial content; earlier entries
			// in this batch already completed and must not be touched, and this
			// call is known to have created (part of) destinations[i] itself
			// (Conflict, meaning another process raced in and created it first,
			// is excluded above), so removing it here can't delete another
			// process's file.
			removeEntryRecursive(QFileInfo(destinations[i]));
		}
		if (status != CopyEntryStatus::Ok) {
			return false;
		}
	}
	return true;
}

bool copyFile(const QString& fromFile, const QString& toFile, WId /*owner*/, TeFileOpProgress* progress)
{
	if (fromFile.isEmpty() || toFile.isEmpty()) {
		return false;
	}
	QFileInfo srcInfo(fromFile);
	if (!srcInfo.exists() && !srcInfo.isSymLink()) {
		return false;
	}
	if (!QFileInfo(QFileInfo(toFile).path()).isDir()) {
		return false;
	}
	if (pathEntryExists(toFile)) {
		return false; // conflict: never overwrite
	}
	if (!srcInfo.isSymLink() && srcInfo.isDir() && isPathWithinOrEqual(fromFile, toFile)) {
		return false; // toFile is fromFile itself, or nested below it: would self-recurse
	}
	if (isSpecialFile(srcInfo)) {
		return false; // FIFOs/sockets/devices cannot be content-copied safely
	}

	if (progress != nullptr) {
		progress->start(computeTotalBytes({ fromFile }, progress));
		if (progress->isCancelled()) {
			return false;
		}
	}
	const CopyEntryStatus status = copyEntryRecursive(srcInfo, toFile, progress);
	if (status == CopyEntryStatus::Failed) {
		// toFile was confirmed not to exist above and this call is known to have
		// created (part of) it itself (Conflict, meaning another process raced
		// in and created toFile first, is excluded above), so any partial
		// content left behind by the failed/cancelled copy is safe to remove.
		removeEntryRecursive(QFileInfo(toFile));
	}
	return status == CopyEntryStatus::Ok;
}

bool moveFiles(const QStringList& files, const QString& path, WId /*owner*/, TeFileOpProgress* progress)
{
	QStringList destinations;
	if (!buildDestinations(files, path, &destinations)) {
		return false;
	}

	QVector<qint64> entrySizes;
	if (progress != nullptr) {
		progress->start(computeEntrySizes(files, progress, &entrySizes));
		if (progress->isCancelled()) {
			return false;
		}
	}

	for (int i = 0; i < files.size(); ++i) {
		if (progress != nullptr && progress->isCancelled()) {
			return false;
		}
		const QString& src = files[i];
		const QString& dest = destinations[i];
		const qint64 entrySize = (progress != nullptr) ? entrySizes[i] : 0;

		// QFile::rename() must only be attempted on the same device: on EXDEV,
		// Qt's own fallback silently performs an internal copy+remove that is not
		// observable here (no progress/cancellation, and it does not go through
		// copyEntryRecursive()'s symlink/special-file handling), so cross-device
		// moves must always use the copy+remove path below instead.
		if (sameDevice(src, path) && QFile::rename(src, dest)) {
			// Same-filesystem rename; treat as instantly processed for progress.
			if (progress != nullptr) {
				progress->addProcessed(entrySize, src);
			}
			continue;
		}

		// Cross-device (or otherwise rename-incapable) fallback: copy then
		// permanently remove the now-duplicated source.
		const CopyEntryStatus status = copyEntryRecursive(QFileInfo(src), dest, progress);
		if (status == CopyEntryStatus::Failed) {
			// The copy failed/was cancelled and the source is still intact, so
			// the (possibly partial) new destination this call created can be
			// safely removed; this call is known to have created (part of) it
			// itself (Conflict, meaning another process raced in and created it
			// first, is excluded above).
			removeEntryRecursive(QFileInfo(dest));
		}
		if (status != CopyEntryStatus::Ok) {
			return false;
		}
		if (!removeEntryRecursive(QFileInfo(src))) {
			// The copy to dest fully succeeded but the source could not be
			// removed. dest may now be the only fully-intact copy (the source
			// removal may have partially deleted it), so leave dest in place
			// rather than risk losing the data; just report failure.
			return false;
		}
	}
	return true;
}

bool deleteFiles(const QStringList& files, WId /*owner*/, TeFileOpProgress* progress)
{
	if (files.isEmpty()) {
		return false;
	}
	for (const QString& f : files) {
		if (f.isEmpty() || !pathEntryExists(f)) {
			return false;
		}
	}

	QVector<qint64> entrySizes;
	if (progress != nullptr) {
		progress->start(computeEntrySizes(files, progress, &entrySizes));
		if (progress->isCancelled()) {
			return false;
		}
	}

	for (int i = 0; i < files.size(); ++i) {
		if (progress != nullptr && progress->isCancelled()) {
			return false;
		}
		const QString& f = files[i];
		const qint64 size = (progress != nullptr) ? entrySizes[i] : 0;
		// Always goes through the trash; never falls back to a permanent
		// delete (important for cross-device entries).
		if (!QFile::moveToTrash(f)) {
			return false;
		}
		if (progress != nullptr) {
			progress->addProcessed(size, f);
		}
	}
	return true;
}

QPixmap getThumbnail(const QString& path, const QSize& size)
{
	QFileInfo info(path);
	if (!info.isFile()) {
		return QPixmap();
	}

	const QString cacheHome = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
	if (cacheHome.isEmpty()) {
		return QPixmap();
	}

	// freedesktop.org thumbnail managing standard: cache key is the MD5 hash
	// of the file's URI, looked up under normal (<=128) or large (<=256). The
	// URI must be the absolute (non-canonical) path: resolving symlinks here
	// would compute a different key than the one other apps store under, so
	// thumbnails would never be found for files accessed via a symlink.
	const QString variant = (size.width() > 128 || size.height() > 128) ? QStringLiteral("large") : QStringLiteral("normal");
	const QUrl uri = QUrl::fromLocalFile(info.absoluteFilePath());
	const QString uriText = uri.toString(QUrl::FullyEncoded);
	const QByteArray hash = QCryptographicHash::hash(uriText.toUtf8(), QCryptographicHash::Md5).toHex();
	const QString thumbPath = QStringLiteral("%1/thumbnails/%2/%3.png").arg(cacheHome, variant, QString::fromLatin1(hash));

	if (!QFileInfo::exists(thumbPath)) {
		return QPixmap(); // no synchronous decode fallback: cache miss means no thumbnail
	}

	QImageReader reader(thumbPath);
	const QString cachedUri = reader.text(QStringLiteral("Thumb::URI"));
	if (!cachedUri.isEmpty() && cachedUri != uriText) {
		return QPixmap(); // MD5 collision or unexpected cache content: not this file
	}
	const QByteArray cachedMTimeText = reader.text(QStringLiteral("Thumb::MTime")).toUtf8();
	if (!cachedMTimeText.isEmpty()) {
		bool ok = false;
		const qint64 cachedMTime = cachedMTimeText.toLongLong(&ok);
		if (ok && cachedMTime != info.lastModified().toSecsSinceEpoch()) {
			return QPixmap(); // stale cache entry
		}
	}

	const QImage image = reader.read();
	if (image.isNull()) {
		return QPixmap();
	}
	// Never upscale beyond the cached thumbnail's own (native) resolution.
	const QSize targetSize = size.boundedTo(image.size());
	return QPixmap::fromImage(image.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap getFileIcon(const QString& path, const QSize& size)
{
	QFileInfo info(path);
	QMimeDatabase mimeDb;
	const QMimeType mime = info.isDir() ? mimeDb.mimeTypeForName(QStringLiteral("inode/directory"))
	                                     : mimeDb.mimeTypeForFile(info);

	// QIcon::fromTheme() alone isn't a reliable "found in theme?" check on all
	// platforms/themes, so hasThemeIcon() gates each lookup before using it.
	QIcon icon;
	if (QIcon::hasThemeIcon(mime.iconName())) {
		icon = QIcon::fromTheme(mime.iconName());
	}
	else if (QIcon::hasThemeIcon(mime.genericIconName())) {
		icon = QIcon::fromTheme(mime.genericIconName());
	}
	else {
		const QString fallbackName = info.isDir() ? QStringLiteral("folder") : QStringLiteral("text-x-generic");
		if (QIcon::hasThemeIcon(fallbackName)) {
			icon = QIcon::fromTheme(fallbackName);
		}
	}
	if (icon.isNull()) {
		return QPixmap();
	}
	return icon.pixmap(size);
}

QString getAssociatedAppPath(const QString& /*suffix*/)
{
	// Deferred: freedesktop "open with" resolution needs a .desktop /
	// mimeapps.list lookup with no direct Qt API; not required for Phase 1.
	return QString();
}

//////////////////////////////////////////////////////////////
//
// Clipboard action
//

namespace {
	// Applies the Windows-style hint (kept for same-app/-toolkit interop) plus
	// the GNOME (Nautilus) and KDE (Dolphin) cut/copy clipboard conventions.
	void setClipboardActionFormats(QMimeData* mime, bool isCut)
	{
		QByteArray dropEffect(4, 0);
		dropEffect[0] = isCut ? 2 : 5;
		mime->setData("Preferred DropEffect", dropEffect);

		mime->setData("application/x-kde-cutselection", isCut ? QByteArray("1") : QByteArray("0"));

		QByteArray gnome = isCut ? QByteArray("cut\n") : QByteArray("copy\n");
		const QList<QUrl> urls = mime->urls();
		for (int i = 0; i < urls.size(); ++i) {
			gnome += urls[i].toString(QUrl::FullyEncoded).toUtf8();
			if (i + 1 < urls.size()) {
				gnome += '\n';
			}
		}
		mime->setData("x-special/gnome-copied-files", gnome);
	}
}

bool isMoveAction(const QMimeData* mime)
{
	if (mime->hasFormat("application/x-qt-windows-mime;value=\"Preferred DropEffect\"")) {
		const QByteArray array = mime->data("application/x-qt-windows-mime;value=\"Preferred DropEffect\"");
		return !array.isEmpty() && (array.at(0) == 2);
	}

	if (mime->hasFormat("Preferred DropEffect")) {
		const QByteArray array = mime->data("Preferred DropEffect");
		return !array.isEmpty() && (array.at(0) == 2);
	}

	// KDE (Dolphin): content "1" means the selection was cut.
	if (mime->hasFormat("application/x-kde-cutselection")) {
		return mime->data("application/x-kde-cutselection").trimmed() == "1";
	}

	// GNOME (Nautilus/Files): first line is "cut" or "copy".
	if (mime->hasFormat("x-special/gnome-copied-files")) {
		return mime->data("x-special/gnome-copied-files").startsWith("cut");
	}

	return false;
}

void setMoveAction(QMimeData* mime)
{
	setClipboardActionFormats(mime, true);
}

void setCopyAction(QMimeData* mime)
{
	setClipboardActionFormats(mime, false);
}

//////////////////////////////////////////////////////////////
//
// NativeEventFilter
//
TeNativeEvent* getNativeEvent()
{
	return &g_event;
}

//////////////////////////////////////////////////////////////
//
// Shell
//
QString getDefaultShellCommand()
{
	return QStringLiteral("/bin/sh");
}

namespace platform_util_test {
namespace {
	QString driveActionKey(const TeDriveAction& action)
	{
		return QString::number(action.text.size()) + QLatin1Char(':') + action.text
			+ QString::number(action.path.size()) + QLatin1Char(':') + action.path
			+ QString::number(action.toolTip.size()) + QLatin1Char(':') + action.toolTip
			+ QString::number(action.identity.size()) + QLatin1Char(':') + action.identity;
	}

	QStringList sortedDriveActionKeys(const QList<TeDriveAction>& actions)
	{
		QStringList keys;
		keys.reserve(actions.size());
		for (const TeDriveAction& action : actions) {
			keys.append(driveActionKey(action));
		}
		std::sort(keys.begin(), keys.end());
		return keys;
	}
}

QString driveActionLabel(int index)
{
	QString label;
	int value = index + 2;
	do {
		label.prepend(QChar(u'A' + (value % 26)));
		value = (value / 26) - 1;
	} while (value >= 0);
	return label;
}

void DriveActionLabelAllocator::assign(QList<TeDriveAction>* actions)
{
	QSet<QString> presentIdentities;
	presentIdentities.reserve(actions->size());
	for (const TeDriveAction& action : *actions) {
		presentIdentities.insert(action.identity);
	}

	for (auto it = m_labelIndices.begin(); it != m_labelIndices.end();) {
		if (!presentIdentities.contains(it.key())) {
			it = m_labelIndices.erase(it);
		} else {
			++it;
		}
	}

	QSet<int> allocatedIndices;
	for (auto it = m_labelIndices.cbegin(); it != m_labelIndices.cend(); ++it) {
		allocatedIndices.insert(it.value());
	}
	for (TeDriveAction& action : *actions) {
		const QString& identity = action.identity;
		int labelIndex = m_labelIndices.value(identity, -1);
		if (labelIndex < 0) {
			labelIndex = 0;
			while (allocatedIndices.contains(labelIndex)) {
				++labelIndex;
			}
			m_labelIndices.insert(identity, labelIndex);
			allocatedIndices.insert(labelIndex);
		}
		action.text = driveActionLabel(labelIndex) + QLatin1Char(':') + action.text;
	}
}

bool isExternalBlockDevice(const QString& blockDevice, const QString& sysfsRoot)
{
	QSet<QString> visited;
	return isExternalBlockDeviceImpl(blockDevice, sysfsRoot, &visited);
}

DriveActionChange compareDriveActionSnapshots(const QList<TeDriveAction>& previous, const QList<TeDriveAction>& current)
{
	const QStringList previousKeys = sortedDriveActionKeys(previous);
	const QStringList currentKeys = sortedDriveActionKeys(current);
	if (previousKeys == currentKeys) {
		return {};
	}

	int previousIndex = 0;
	int currentIndex = 0;
	while (previousIndex < previousKeys.size() && currentIndex < currentKeys.size()) {
		if (previousKeys[previousIndex] == currentKeys[currentIndex]) {
			++previousIndex;
			++currentIndex;
		} else if (previousKeys[previousIndex] < currentKeys[currentIndex]) {
			++previousIndex;
		} else {
			return { true, true };
		}
	}
	return { true, currentIndex < currentKeys.size() };
}
}

//////////////////////////////////////////////////////////////
//
// Colour scheme
//
TeStyleColorScheme getStyleColorScheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	const QStyleHints* hints = QGuiApplication::styleHints();
	if (hints) {
		switch (hints->colorScheme()) {
		case Qt::ColorScheme::Dark:
			return TeStyleColorScheme::Dark;
		case Qt::ColorScheme::Light:
			return TeStyleColorScheme::Light;
		default:
			break;
		}
	}
#endif
	return TeStyleColorScheme::Light;
}
