#pragma once

#include <QAbstractFileIconProvider>
#include <QIcon>

#include <memory>

/**
 * @file TeFileIconProvider.h
 * @brief Icon provider that avoids per-file shell access on WSL / network / OneDrive paths.
 * @ingroup utility
 */

/**
 * @class TeFileIconProvider
 * @brief Delegates to a fallback provider unless TeRemoteAccess suppresses icon access.
 * @ingroup utility
 *
 * @details Installed on QFileSystemModel before its root path is set.  QFileSystemModel
 * calls icon() and type() from its gatherer thread, so the provider only reads
 * lock-free policy state and pre-built generic icons.  Construct the shared
 * instance on the GUI thread.
 *
 * @see TeRemoteAccess
 */
class TeFileIconProvider : public QAbstractFileIconProvider
{
public:
	/** @param fallback Provider used when icon access is not suppressed; defaults to QFileIconProvider.
	 *  @param foldersOnly True for directory-only models; suppressed paths then never call QFileInfo::isDir(). */
	explicit TeFileIconProvider(std::unique_ptr<QAbstractFileIconProvider> fallback = nullptr, bool foldersOnly = false);
	~TeFileIconProvider() override;

	using QAbstractFileIconProvider::icon;
	QIcon icon(IconType type) const override;
	QIcon icon(const QFileInfo& info) const override;
	QString type(const QFileInfo& info) const override;

	/**
	 * @brief Icon for a folder or drive root without touching the file system when suppressed.
	 * @param path Folder or drive path.
	 */
	QIcon folderIcon(const QString& path) const;

	/** @brief Returns the shared instance (create it on the GUI thread). */
	static TeFileIconProvider* instance();

	/** @brief Shared instance for directory-only models (e.g. the folder tree). */
	static TeFileIconProvider* folderInstance();

private:
	bool m_foldersOnly;
	std::unique_ptr<QAbstractFileIconProvider> mp_fallback;
	QIcon m_folder;
	QIcon m_file;
	QIcon m_drive;
};
