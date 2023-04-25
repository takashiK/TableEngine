#include "TeThumbnailProvider.h"

TeThumbnailProvider::TeThumbnailProvider()
{
}

TeThumbnailProvider::~TeThumbnailProvider()
{
}

QIcon TeThumbnailProvider::icon(const QFileInfo& info) const
{
    return QAbstractFileIconProvider::icon(info);
}
