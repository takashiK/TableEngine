#include "TeThumbnailProvider.h"
#include "TeFileIconProvider.h"
#include "TeRemoteAccess.h"
#include "platform/platform_util.h"


#include <QImageReader>

/**
 * @file TeThumbnailProvider.cpp
 * @brief Implementation of TeThumbnailProvider.
 * @ingroup utility
 */

/**
 * @file TeThumbnailProvider.cpp
 * @brief Implementation of TeThumbnailProvider.
 * @ingroup utility
 */

TeThumbnailProvider::TeThumbnailProvider()
{
}

TeThumbnailProvider::~TeThumbnailProvider()
{
}

QIcon TeThumbnailProvider::icon(const QFileInfo& info) const
{
    // Placeholder icons must not trigger shell thumbnail extraction on remote paths.
    if (TeRemoteAccess::suppressIconAccess(info.filePath()))
        return TeFileIconProvider::instance()->icon(QAbstractFileIconProvider::File);

    if (info.isFile()) {
		QPixmap thumb = getThumbnail(info.absoluteFilePath(), QSize(128, 128));
		if (!thumb.isNull()) {
			return QIcon(thumb);
		}
	}
    return QAbstractFileIconProvider::icon(info);
}

QAbstractFileIconProvider* TeThumbnailProvider::iconProvider()
{
    static TeThumbnailProvider iconProvider;
    return &iconProvider;
}
