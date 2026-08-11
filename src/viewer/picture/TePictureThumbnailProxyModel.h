#pragma once

#include "TeThumbnailPixmapCache.h"

#include <QFileInfo>
#include <QPersistentModelIndex>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QThreadPool>
#include <QTimer>

class QImage;
class QPixmap;

class TePictureThumbnailLoadTask;
class tst_TePictureThumbnailProxyModel;

class TePictureThumbnailProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	enum Roles {
		ThumbnailPixmap = Qt::UserRole + 60, ///< Decoded thumbnail pixmap, invalid while not available.
	};

	explicit TePictureThumbnailProxyModel(QObject* parent = nullptr);
	~TePictureThumbnailProxyModel() override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QSize thumbnailSize() const;
	void clearThumbnailCache();
	void setThumbnailLoadingEnabled(bool enabled);

private:
	void thumbnailLoaded(quint64 generation, const QString& cacheKey, const QString& filePath,
		const QImage& image);
	void flushPendingUpdates();
	QString cacheKey(const QFileInfo& info) const;
	static bool isSupportedImageFile(const QFileInfo& info);

	QSize m_thumbnailSize{96, 96};
	mutable TeThumbnailPixmapCache m_thumbnailCache;
	mutable QSet<QString> m_pendingRequests;
	mutable QSet<QPersistentModelIndex> m_deferredIndexes;
	QSet<QPersistentModelIndex> m_updatedIndexes;
	mutable QThreadPool m_thumbnailThreadPool;
	QTimer m_updateTimer;
	mutable int m_requestPriority = 0;
	quint64 m_generation = 0;
	bool m_thumbnailLoadingEnabled = true;

	friend class TePictureThumbnailLoadTask;
	friend class tst_TePictureThumbnailProxyModel;
};
