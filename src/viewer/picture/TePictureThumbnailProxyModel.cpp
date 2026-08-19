#include "TePictureThumbnailProxyModel.h"
#include "utils/TeEmbeddedImageLoader.h"

#include <QFileSystemModel>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QMetaObject>
#include <QPointer>
#include <QThread>
#include <QThreadPool>

#include <algorithm>
#include <utility>

namespace
{
constexpr int UpdateCoalescingIntervalMs = 50;
constexpr int MaxThumbnailThreads = 4;
}

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
		const QImage image = teDecodeEmbeddedImage(m_filePath, m_thumbnailSize);

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
	m_thumbnailThreadPool.setMaxThreadCount(qBound(1, QThread::idealThreadCount() / 2, MaxThumbnailThreads));
	m_thumbnailThreadPool.setThreadPriority(QThread::LowPriority);

	m_updateTimer.setSingleShot(true);
	m_updateTimer.setInterval(UpdateCoalescingIntervalMs);
	connect(&m_updateTimer, &QTimer::timeout, this, &TePictureThumbnailProxyModel::flushPendingUpdates);
}

TePictureThumbnailProxyModel::~TePictureThumbnailProxyModel()
{
	++m_generation;
	m_thumbnailThreadPool.clear();
	m_thumbnailThreadPool.waitForDone();
}

QVariant TePictureThumbnailProxyModel::data(const QModelIndex& index, int role) const
{
	if (role != ThumbnailPixmap)
		return QSortFilterProxyModel::data(index, role);

	const auto* fileSystemModel = qobject_cast<const QFileSystemModel*>(sourceModel());
	if (fileSystemModel == nullptr)
		return QVariant();

	const QFileInfo info = fileSystemModel->fileInfo(mapToSource(index));
	if (!isSupportedImageFile(info))
		return QVariant();

	const QString key = cacheKey(info);
	if (const QPixmap* cached = m_thumbnailCache.find(key))
		return *cached;

	if (!m_thumbnailLoadingEnabled) {
		m_deferredIndexes.insert(index);
		return QVariant();
	}

	if (!m_pendingRequests.contains(key)) {
		m_pendingRequests.insert(key);
		auto* task = new TePictureThumbnailLoadTask(key, info.absoluteFilePath(), m_thumbnailSize, m_generation,
			const_cast<TePictureThumbnailProxyModel*>(this));
		task->setAutoDelete(true);
		// Requests are issued while painting, so the newest request is the one on screen.
		m_thumbnailThreadPool.start(task, ++m_requestPriority);
	}

	return QVariant();
}

QSize TePictureThumbnailProxyModel::thumbnailSize() const
{
	return m_thumbnailSize;
}

void TePictureThumbnailProxyModel::clearThumbnailCache()
{
	++m_generation;
	m_thumbnailCache.clear();
	m_pendingRequests.clear();
	m_deferredIndexes.clear();
	m_updatedIndexes.clear();
	m_updateTimer.stop();
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
	m_updatedIndexes.unite(deferredIndexes);
	flushPendingUpdates();
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

	const QModelIndex proxyIndex = mapFromSource(sourceFileModel->index(filePath));
	if (!proxyIndex.isValid())
		return;

	m_updatedIndexes.insert(proxyIndex);
	if (!m_updateTimer.isActive())
		m_updateTimer.start();
}

void TePictureThumbnailProxyModel::flushPendingUpdates()
{
	m_updateTimer.stop();
	const auto updatedIndexes = std::exchange(m_updatedIndexes, {});

	QHash<QModelIndex, QList<int>> rowsByParent;
	for (const QPersistentModelIndex& updated : updatedIndexes) {
		if (updated.isValid())
			rowsByParent[updated.parent()].append(updated.row());
	}

	for (auto it = rowsByParent.begin(); it != rowsByParent.end(); ++it) {
		QList<int>& rows = it.value();
		std::sort(rows.begin(), rows.end());
		int first = rows.constFirst();
		int last = first;
		for (int i = 1; i < rows.size(); ++i) {
			if (rows.at(i) == last + 1) {
				last = rows.at(i);
				continue;
			}
			emit dataChanged(index(first, 0, it.key()), index(last, 0, it.key()), {ThumbnailPixmap});
			first = rows.at(i);
			last = first;
		}
		emit dataChanged(index(first, 0, it.key()), index(last, 0, it.key()), {ThumbnailPixmap});
	}
}

bool TePictureThumbnailProxyModel::isSupportedImageFile(const QFileInfo& info)
{
	static const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
	return info.isFile() && supportedFormats.contains(info.suffix().toLower().toUtf8());
}

QString TePictureThumbnailProxyModel::cacheKey(const QFileInfo& info) const
{
	return QStringLiteral("%1|%2x%3|%4")
		.arg(info.absoluteFilePath())
		.arg(m_thumbnailSize.width())
		.arg(m_thumbnailSize.height())
		.arg(info.lastModified().toMSecsSinceEpoch());
}