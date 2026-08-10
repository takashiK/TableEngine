#include "TePictureThumbnailProxyModel.h"

#include <QApplication>
#include <QFileSystemModel>
#include <QImageReader>
#include <QMetaObject>
#include <QPointer>
#include <QStyle>
#include <QThreadPool>

#include <utility>

class TePictureThumbnailLoadTask : public QRunnable
{
public:
	TePictureThumbnailLoadTask(const QString& cacheKey, const QString& filePath,
		const QSize& thumbnailSize, quint64 generation, TePictureThumbnailProxyModel* model)
		: m_cacheKey(cacheKey)
		, m_filePath(filePath)
		, m_thumbnailSize(thumbnailSize)
		, m_generation(generation)
		, mp_model(model)
	{
	}

	void run() override
	{
		QImageReader reader(m_filePath);
		reader.setAutoTransform(true);
		const QSize sourceSize = reader.size();
		if (sourceSize.isValid()
			&& (sourceSize.width() > m_thumbnailSize.width() || sourceSize.height() > m_thumbnailSize.height())) {
			reader.setScaledSize(sourceSize.scaled(m_thumbnailSize, Qt::KeepAspectRatio));
		}
		QImage image = reader.read();
		if (!image.isNull() && (image.width() > m_thumbnailSize.width() || image.height() > m_thumbnailSize.height())) {
			image = image.scaled(m_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		const QPointer<TePictureThumbnailProxyModel> model = mp_model;
		if (!model)
			return;

		QMetaObject::invokeMethod(model, [model, generation = m_generation, cacheKey = m_cacheKey, filePath = m_filePath, image]() {
			if (model)
				model->thumbnailLoaded(generation, cacheKey, filePath, image);
		}, Qt::QueuedConnection);
	}

private:
	QString m_cacheKey;
	QString m_filePath;
	QSize m_thumbnailSize;
	quint64 m_generation;
	QPointer<TePictureThumbnailProxyModel> mp_model;
};

TePictureThumbnailProxyModel::TePictureThumbnailProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
{
	m_thumbnailThreadPool.setMaxThreadCount(1);
	m_thumbnailThreadPool.setThreadPriority(QThread::LowPriority);
}

TePictureThumbnailProxyModel::~TePictureThumbnailProxyModel()
{
	++m_generation;
	m_thumbnailThreadPool.clear();
	m_thumbnailThreadPool.waitForDone();
}

QVariant TePictureThumbnailProxyModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DecorationRole)
		return QSortFilterProxyModel::data(index, role);

	const QModelIndex sourceIndex = mapToSource(index);
	const QVariant infoValue = sourceModel()->data(sourceIndex, QFileSystemModel::FileInfoRole);
	if (!infoValue.isValid())
		return QSortFilterProxyModel::data(index, role);

	const QFileInfo info = qvariant_cast<QFileInfo>(infoValue);
	if (!info.isFile() || QImageReader::imageFormat(info.absoluteFilePath()).isEmpty())
		return QSortFilterProxyModel::data(index, role);

	if (!m_thumbnailLoadingEnabled) {
		m_deferredIndexes.insert(index);
		return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
	}

	const QString key = cacheKey(info);
	const QPixmap* cached = m_thumbnailCache.find(key);
	if (cached)
		return *cached;

	if (!m_pendingRequests.contains(key)) {
		m_pendingRequests.insert(key);
		auto* task = new TePictureThumbnailLoadTask(key, info.absoluteFilePath(), m_thumbnailSize, m_generation,
			const_cast<TePictureThumbnailProxyModel*>(this));
		task->setAutoDelete(true);
		m_thumbnailThreadPool.start(task);
	}

	return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

void TePictureThumbnailProxyModel::clearThumbnailCache()
{
	++m_generation;
	m_thumbnailCache.clear();
	m_pendingRequests.clear();
	m_deferredIndexes.clear();
}

void TePictureThumbnailProxyModel::setThumbnailLoadingEnabled(bool enabled)
{
	if (m_thumbnailLoadingEnabled == enabled)
		return;

	m_thumbnailLoadingEnabled = enabled;
	if (!enabled) {
		++m_generation;
		m_thumbnailThreadPool.clear();
		m_pendingRequests.clear();
		return;
	}

	const auto deferredIndexes = std::exchange(m_deferredIndexes, {});
	for (const QPersistentModelIndex& index : deferredIndexes) {
		if (index.isValid())
			emit dataChanged(index, index, {Qt::DecorationRole});
	}
}

void TePictureThumbnailProxyModel::thumbnailLoaded(quint64 generation, const QString& key, const QString& filePath,
	const QImage& image)
{
	if (generation != m_generation)
		return;

	if (!m_pendingRequests.remove(key))
		return;

	if (image.isNull())
		return;

	m_thumbnailCache.insert(key, QPixmap::fromImage(image), image);
	auto* sourceFileModel = qobject_cast<QFileSystemModel*>(sourceModel());
	if (!sourceFileModel)
		return;

	const QModelIndex sourceIndex = sourceFileModel->index(filePath);
	const QModelIndex proxyIndex = mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		emit dataChanged(proxyIndex, proxyIndex, {Qt::DecorationRole});
}

QString TePictureThumbnailProxyModel::cacheKey(const QFileInfo& info) const
{
	return QStringLiteral("%1|%2x%3|%4")
		.arg(info.absoluteFilePath())
		.arg(m_thumbnailSize.width())
		.arg(m_thumbnailSize.height())
		.arg(info.lastModified().toMSecsSinceEpoch());
}