#pragma once

#include "TeThumbnailPixmapCache.h"

#include <QFileInfo>
#include <QPersistentModelIndex>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QThreadPool>

class QImage;
class QPixmap;

class TePictureThumbnailLoadTask;
class tst_TePictureThumbnailProxyModel;

class TePictureThumbnailProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit TePictureThumbnailProxyModel(QObject* parent = nullptr);
	~TePictureThumbnailProxyModel() override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	void clearThumbnailCache();
	void setThumbnailLoadingEnabled(bool enabled);

private:
	void thumbnailLoaded(quint64 generation, const QString& cacheKey, const QString& filePath,
		const QImage& image);
	QString cacheKey(const QFileInfo& info) const;

	QSize m_thumbnailSize{96, 96};
	mutable TeThumbnailPixmapCache m_thumbnailCache;
	mutable QSet<QString> m_pendingRequests;
	mutable QSet<QPersistentModelIndex> m_deferredIndexes;
	mutable QThreadPool m_thumbnailThreadPool;
	quint64 m_generation = 0;
	bool m_thumbnailLoadingEnabled = true;

	friend class TePictureThumbnailLoadTask;
	friend class tst_TePictureThumbnailProxyModel;
};
