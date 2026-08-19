#pragma once

#include <QString>

/**
 * @file TeAppPaths.h
 * @brief Resolves writable locations for user-customisable application assets.
 * @ingroup utility
 */

/**
 * @brief Directory used to store/read user-writable app assets (syntax
 * highlight definitions, user stylesheet overrides).
 * @ingroup utility
 *
 * @details On Windows this is applicationDirPath(), preserving the existing
 * portable install layout. On Linux, applicationDirPath() (typically /usr/bin)
 * is not writable, so a per-user XDG data directory
 * (QStandardPaths::AppDataLocation) is used instead. The directory itself is
 * created once and cached, but any default highlight file missing from it is
 * copied from the read-only system defaults on every call (without ever
 * overwriting an existing, possibly user-edited, file).
 * @return Absolute path to the user asset directory (created if missing).
 */
QString teUserAssetDir();
