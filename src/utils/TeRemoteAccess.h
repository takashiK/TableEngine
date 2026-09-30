#pragma once

#include "TeSettings.h"

#include <QString>
#include <QStringList>

#include <functional>

/**
 * @file TeRemoteAccess.h
 * @brief Per-kind (WSL / network share / OneDrive) access policy for file browsing.
 * @ingroup utility
 */

/**
 * @namespace TeRemoteAccess
 * @brief Classifies paths by storage kind and resolves the configured access mode.
 * @ingroup utility
 *
 * @details Each remote kind has an independent TeSettings::RemoteAccessMode:
 * - SUPPRESS_ICON: no per-file shell icon/type lookups (generic icons only).
 * - SUPPRESS_AUTO: additionally no automatic thumbnail / detail-panel content reads.
 * - NORMAL: unchanged legacy behaviour.
 *
 * Local paths are always NORMAL.  Mode reads are lock-free and safe from the
 * QFileSystemModel gatherer thread.
 */
namespace TeRemoteAccess {

	enum class Kind {
		Local = 0,
		Wsl,
		Network,
		OneDrive,
	};

	inline constexpr int KIND_COUNT = 4;

	/** @brief Outcome of probing a Windows drive letter. */
	struct DriveResolution {
		Kind kind = Kind::Local;
		bool cacheable = true; ///< False when the probe result is uncertain (e.g. disconnected mapping).
	};

	/**
	 * @brief Pure interpretation of GetDriveType() / WNetGetConnection() results.
	 * @param driveType        GetDriveType() value (0 unknown, 1 no root dir, 4 remote).
	 * @param connectionResult WNetGetConnection() return code (0 success, 1201 connection unavailable).
	 * @param remoteName       Remote name reported by WNetGetConnection() (may be empty).
	 */
	DriveResolution resolveDriveProbe(unsigned driveType, unsigned connectionResult, const QString& remoteName);

	/** @brief True when @p path is a lexical filesystem root ("C:", "C:/", "/", UNC share root); performs no I/O. */
	bool isLexicalRoot(const QString& path);

	/** @brief How long an uncertain (non-cacheable) drive probe result is reused. */
	inline constexpr qint64 DRIVE_UNCERTAIN_TTL_MS = 2000;

	/**
	 * @brief Resolves @p letter through the drive caches, calling @p probe only on a miss.
	 * @details Certain results are cached until clearDriveCache(); uncertain results are reused
	 *          for DRIVE_UNCERTAIN_TTL_MS so sort/paint does not re-probe a disconnected drive.
	 * @param nowMs Monotonic clock in milliseconds.
	 */
	Kind resolveDriveCached(QChar letter, const std::function<DriveResolution(QChar)>& probe, qint64 nowMs);

	/** @brief Resolves a Windows drive letter (upper case) to Local / Wsl / Network. */
	using DriveKindResolver = std::function<Kind(QChar driveLetter)>;

	/**
	 * @brief Pure path classification.
	 * @param path          Path to classify (native or generic separators).
	 * @param resolveDrive  Resolver for drive-letter paths; null treats them as Local.
	 * @param oneDriveRoots Configured OneDrive root folders; only paths under these roots are OneDrive.
	 */
	Kind classifyPath(const QString& path, const DriveKindResolver& resolveDrive, const QStringList& oneDriveRoots);

	/** @brief Classifies @p path using the OS drive resolver and OneDrive environment roots. */
	Kind classify(const QString& path);

	/** @brief Forgets cached drive-letter classification and OneDrive roots (call when mappings may have changed). */
	void clearDriveCache();

	/** @brief Default mode applied to WSL / network / OneDrive kinds. */
	TeSettings::RemoteAccessMode defaultMode();

	/** @brief Returns the configured mode for @p kind (Local is always NORMAL). */
	TeSettings::RemoteAccessMode mode(Kind kind);
	void setMode(Kind kind, TeSettings::RemoteAccessMode mode);

	/** @brief Resets all kinds to defaultMode(). */
	void resetModes();

	/** @brief Returns the QSettings key that stores the mode for @p kind, or nullptr for Local. */
	const char* settingKey(Kind kind);

	/**
	 * @brief Loads modes from QSettings.
	 * @return true when at least one mode changed.
	 */
	bool loadFromSettings();

	/** @brief Returns the mode governing @p path. */
	TeSettings::RemoteAccessMode modeForPath(const QString& path);

	/** @brief True when per-file shell icon/type lookups must be avoided for @p path. */
	bool suppressIconAccess(const QString& path);

	/** @brief True when automatic thumbnail/preview content reads must be avoided for @p path. */
	bool suppressAutoContent(const QString& path);

	/**
	 * @brief Overrides classify() globally; pass an empty function to restore the default.
	 * @note Intended for tests only.
	 */
	void setClassifierForTesting(std::function<Kind(const QString&)> classifier);
}
