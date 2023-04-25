#pragma once

#include <QAbstractFileIconProvider>

class TeThumbnailProvider : public QAbstractFileIconProvider
{
public:
	TeThumbnailProvider();
	virtual ~TeThumbnailProvider();
	virtual QIcon icon(const QFileInfo& info) const override;
};

