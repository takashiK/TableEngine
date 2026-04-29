#pragma once

#include <QAbstractFileIconProvider>

/**
 * @file TeThumbnailProvider.h
 * @brief QAbstractFileIconProvider subclass that returns OS-cached thumbnails.
 * @ingroup utility
 */

/**
 * @class TeThumbnailProvider
 * @brief File icon provider that fetches thumbnail images from the OS shell
 *        thumbnail cache instead of generic type icons.
 * @ingroup utility
 *
 * @details Overrides icon(QFileInfo) to query the Windows shell thumbnail
 * cache (IShellItemImageFactory).  Falls back to the default provider icon
 * when no cached thumbnail is available.
 *
 * The singleton accessor iconProvider() returns a shared instance so that
 * multiple models can share one provider without constructing their own.
 *
 * @see TeFileSortProxyModel
 */
class TeThumbnailProvider : public QAbstractFileIconProvider
{
public:
	TeThumbnailProvider();
	virtual ~TeThumbnailProvider();

	/**
	 * @brief Returns a thumbnail icon for @p info from the OS cache.
	 * @param info File-system entry to fetch the icon for.
	 * @return A QIcon loaded from the OS thumbnail cache, or a generic icon.
	 */
	virtual QIcon icon(const QFileInfo& info) const override;

	/**
	 * @brief Returns the shared TeThumbnailProvider singleton.
	 * @return Pointer to the singleton instance (not owned by the caller).
	 */
	static QAbstractFileIconProvider* iconProvider();
};

