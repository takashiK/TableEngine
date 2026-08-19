#include "TeAppPaths.h"

#include <QApplication>

#ifndef Q_OS_WIN
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace {

// Resolves the read-only system default highlight directory, checked in
// priority order so both a direct run from the build tree and an installed
// package (DEB under /usr, or a custom --prefix) find their assets:
//  1. Next to the running executable (build-tree / portable convention,
//     populated by main/CMakeLists.txt's POST_BUILD copy step).
//  2. Relative to the executable for FHS installs (bindir/../share), which
//     covers both a custom --prefix and, since CPACK_PACKAGING_INSTALL_PREFIX
//     is forced to /usr, the standard Debian package layout too.
//  3. The stable Debian package location, kept as a last-resort fallback in
//     case applicationDirPath() is reached through a symlink/wrapper.
QString findSystemHighlightDir()
{
	const QString appDir = QApplication::applicationDirPath();
	const QStringList candidates = {
		appDir + QStringLiteral("/highlight"),
		appDir + QStringLiteral("/../share/tableengine/highlight"),
		QStringLiteral("/usr/share/tableengine/highlight"),
	};

	for (const QString& candidate : candidates) {
		if (!candidate.isEmpty() && QDir(candidate).exists()) {
			return candidate;
		}
	}
	return QString();
}

} // namespace
#endif

/**
 * @file TeAppPaths.cpp
 * @brief Definition of teUserAssetDir().
 * @ingroup utility
 */

QString teUserAssetDir()
{
#ifdef Q_OS_WIN
	return QApplication::applicationDirPath();
#else
	// The base directory and its "highlight" subfolder only need to be created
	// once per process; cache that part so repeated calls skip the mkpath()s.
	static const QString dir = [] {
		const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

		QDir dataDir(path);
		if (!dataDir.exists()) {
			dataDir.mkpath(QStringLiteral("."));
		}

		QDir highlight(path + QStringLiteral("/highlight"));
		if (!highlight.exists()) {
			highlight.mkpath(QStringLiteral("."));
		}

		return path;
	}();

	// Unlike the directory creation above, copy any default highlight file
	// still missing from the user directory on every call (not just once), so
	// files added by a later app version get picked up for existing users too.
	// Existing files -- including user-edited ones -- are never overwritten.
	const QString sysHighlightDir = findSystemHighlightDir();
	if (!sysHighlightDir.isEmpty()) {
		const QString highlightDir = dir + QStringLiteral("/highlight");
		const QFileInfoList entries = QDir(sysHighlightDir).entryInfoList(QDir::Files);
		for (const QFileInfo& entry : entries) {
			const QString destPath = highlightDir + QStringLiteral("/") + entry.fileName();
			if (!QFile::exists(destPath)) {
				QFile::copy(entry.filePath(), destPath);
			}
		}
	}

	return dir;
#endif
}

