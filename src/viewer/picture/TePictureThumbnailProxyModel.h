#pragma once

#include "TeThumbnailPixmapCache.h"

#include <QFileInfo>
#include <QSet>
#include <QSortFilterProxyModel>

class QImage;
class QPixmap;

class TePictureThumbnailLoadTask;

class TePictureThumbnailProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit TePictureThumbnailProxyModel(QObject* parent = nullptr);

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	void clearThumbnailCache();

private:
	void thumbnailLoaded(quint64 generation, const QString& cacheKey, const QString& filePath,
		const QImage& image);
	QString cacheKey(const QFileInfo& info) const;

	QSize m_thumbnailSize{96, 96};
	mutable TeThumbnailPixmapCache m_thumbnailCache;
	mutable QSet<QString> m_pendingRequests;
	quint64 m_generation = 0;

	friend class TePictureThumbnailLoadTask;
};