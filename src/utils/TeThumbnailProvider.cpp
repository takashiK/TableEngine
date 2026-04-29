#include "TeThumbnailProvider.h"
#include "platform/platform_util.h"


#include <QImageReader>

TeThumbnailProvider::TeThumbnailProvider()
{
}

TeThumbnailProvider::~TeThumbnailProvider()
{
}

QIcon TeThumbnailProvider::icon(const QFileInfo& info) const
{
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
