#include <gmock/gmock.h>
#include <QFile>
#include <QFileSystemModel>
#include <QImage>
#include <QPixmapCache>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

#include "utils/TeImageLoader.h"
#include "viewer/picture/TePictureThumbnailProxyModel.h"
#include "viewer/picture/TeThumbnailPixmapCache.h"

class tst_TePictureThumbnailProxyModel : public ::testing::Test
{
protected:
	static int thumbnailPoolMaxThreadCount(const TePictureThumbnailProxyModel& model)
	{
		return model.m_thumbnailThreadPool.maxThreadCount();
	}

	static QThread::Priority thumbnailPoolThreadPriority(const TePictureThumbnailProxyModel& model)
	{
		return model.m_thumbnailThreadPool.threadPriority();
	}

	static int activeThumbnailThreadCount(const TePictureThumbnailProxyModel& model)
	{
		return model.m_thumbnailThreadPool.activeThreadCount();
	}

	static bool hasPendingThumbnailRequests(const TePictureThumbnailProxyModel& model)
	{
		return !model.m_pendingRequests.isEmpty();
	}

	static qint64 billedBytes(const QPixmap& pixmap, const QImage& image)
	{
		const qint64 bytes = TeThumbnailPixmapCache::estimatedBytes(pixmap, image);
		return ((bytes + 1023) / 1024) * 1024;
	}

	QString writeImage(const QString& name) const
	{
		const QString path = m_directory.path() + QLatin1Char('/') + name;
		QImage image(128, 64, QImage::Format_RGB32);
		image.fill(Qt::red);
		image.save(path, "PNG");
		return path;
	}

	QString writeTextFile(const QString& name) const
	{
		const QString path = m_directory.path() + QLatin1Char('/') + name;
		QFile file(path);
		if (file.open(QIODevice::WriteOnly)) {
			file.write("not an image");
			file.close();
		}
		return path;
	}

	QTemporaryDir m_directory;
};

TEST_F(tst_TePictureThumbnailProxyModel, thumbnail_pool_is_bounded_and_low_priority)
{
	const TePictureThumbnailProxyModel model;

	EXPECT_GE(thumbnailPoolMaxThreadCount(model), 1);
	EXPECT_LE(thumbnailPoolMaxThreadCount(model), 4);
	EXPECT_EQ(thumbnailPoolThreadPriority(model), QThread::LowPriority);
}

TEST_F(tst_TePictureThumbnailProxyModel, thumbnail_cache_evicts_least_recently_used_item)
{
	const QImage image(32, 32, QImage::Format_ARGB32);
	const QPixmap pixmap = QPixmap::fromImage(image);
	TeThumbnailPixmapCache cache(2 * billedBytes(pixmap, image));

	ASSERT_TRUE(cache.insert("first", pixmap, image));
	ASSERT_TRUE(cache.insert("second", pixmap, image));
	ASSERT_NE(cache.find("first"), nullptr);
	ASSERT_TRUE(cache.insert("third", pixmap, image));

	EXPECT_NE(cache.find("first"), nullptr);
	EXPECT_EQ(cache.find("second"), nullptr);
	EXPECT_NE(cache.find("third"), nullptr);
	EXPECT_LE(cache.usedBytes(), cache.budgetBytes());
}

TEST_F(tst_TePictureThumbnailProxyModel, thumbnail_cache_respects_budget_and_rejects_oversized_item)
{
	const QImage image(32, 32, QImage::Format_ARGB32);
	const QPixmap pixmap = QPixmap::fromImage(image);
	const qint64 estimatedBytes = TeThumbnailPixmapCache::estimatedBytes(pixmap, image);
	TeThumbnailPixmapCache cache(estimatedBytes - 1);

	EXPECT_EQ(cache.budgetBytes(), estimatedBytes - 1);
	EXPECT_FALSE(cache.insert("oversized", pixmap, image));
	EXPECT_EQ(cache.find("oversized"), nullptr);
	EXPECT_LE(cache.usedBytes(), cache.budgetBytes());

	TeThumbnailPixmapCache defaultCache;
	EXPECT_EQ(defaultCache.budgetBytes(), 64LL * 1024 * 1024);
}

TEST_F(tst_TePictureThumbnailProxyModel, decoration_shows_file_type_icon_while_thumbnail_loads)
{
	ASSERT_TRUE(m_directory.isValid());
	const QString path = writeImage("thumbnail.png");
	QPixmapCache::clear();

	QFileSystemModel sourceModel;
	sourceModel.setRootPath(m_directory.path());
	ASSERT_TRUE(sourceModel.index(path).isValid());

	TePictureThumbnailProxyModel model;
	model.setSourceModel(&sourceModel);
	// QFileSystemModel populates asynchronously and drops the proxy mapping, so plain indexes go stale.
	const QPersistentModelIndex index = model.mapFromSource(sourceModel.index(path));
	ASSERT_TRUE(index.isValid());

	QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);
	// The file type icon of the source model stays available without waiting for any decoding.
	EXPECT_TRUE(model.data(index, Qt::DecorationRole).canConvert<QIcon>());
	EXPECT_FALSE(model.data(index, TePictureThumbnailProxyModel::ThumbnailPixmap).isValid());
	ASSERT_TRUE(QTest::qWaitFor(
		[&] { return model.data(index, TePictureThumbnailProxyModel::ThumbnailPixmap).canConvert<QPixmap>(); },
		5000));
	// dataChanged is coalesced by a timer, so it lags behind the cached thumbnail becoming available.
	ASSERT_TRUE(QTest::qWaitFor([&] { return !dataChangedSpy.isEmpty(); }, 5000));

	const QVariant thumbnail = model.data(index, TePictureThumbnailProxyModel::ThumbnailPixmap);
	ASSERT_TRUE(thumbnail.canConvert<QPixmap>());
	const QPixmap thumbnailPixmap = qvariant_cast<QPixmap>(thumbnail);
	EXPECT_FALSE(thumbnailPixmap.isNull());
	EXPECT_EQ(thumbnailPixmap.size(), QSize(96, 48));
	EXPECT_TRUE(model.data(index, Qt::DecorationRole).canConvert<QIcon>());

	QPixmap fileListThumbnail;
	EXPECT_FALSE(QPixmapCache::find(
		TeImageLoader::cacheKey(path, QSize(96, 96), QFileInfo(path).lastModified()),
		&fileListThumbnail));
}

TEST_F(tst_TePictureThumbnailProxyModel, thumbnail_is_not_requested_for_unsupported_extension)
{
	ASSERT_TRUE(m_directory.isValid());
	const QString path = writeTextFile("document.txt");

	QFileSystemModel sourceModel;
	sourceModel.setRootPath(m_directory.path());
	ASSERT_TRUE(sourceModel.index(path).isValid());

	TePictureThumbnailProxyModel model;
	model.setSourceModel(&sourceModel);
	const QModelIndex index = model.mapFromSource(sourceModel.index(path));
	ASSERT_TRUE(index.isValid());

	EXPECT_FALSE(model.data(index, TePictureThumbnailProxyModel::ThumbnailPixmap).isValid());
	EXPECT_FALSE(hasPendingThumbnailRequests(model));
}

TEST_F(tst_TePictureThumbnailProxyModel, thumbnail_waits_until_thumbnail_loading_is_enabled)
{
	ASSERT_TRUE(m_directory.isValid());
	const QString path = writeImage("deferred-thumbnail.png");

	QFileSystemModel sourceModel;
	sourceModel.setRootPath(m_directory.path());
	ASSERT_TRUE(sourceModel.index(path).isValid());

	TePictureThumbnailProxyModel model;
	model.setSourceModel(&sourceModel);
	const QModelIndex index = model.mapFromSource(sourceModel.index(path));
	ASSERT_TRUE(index.isValid());

	model.setThumbnailLoadingEnabled(false);
	QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);
	EXPECT_TRUE(model.data(index, Qt::DecorationRole).canConvert<QIcon>());
	EXPECT_FALSE(model.data(index, TePictureThumbnailProxyModel::ThumbnailPixmap).isValid());
	EXPECT_EQ(activeThumbnailThreadCount(model), 0);
	EXPECT_FALSE(hasPendingThumbnailRequests(model));

	model.setThumbnailLoadingEnabled(true);
	ASSERT_EQ(dataChangedSpy.count(), 1);
	const QPersistentModelIndex refreshedIndex = qvariant_cast<QModelIndex>(dataChangedSpy.takeFirst().at(0));
	ASSERT_TRUE(refreshedIndex.isValid());
	EXPECT_TRUE(model.data(refreshedIndex, Qt::DecorationRole).canConvert<QIcon>());
	ASSERT_TRUE(QTest::qWaitFor(
		[&] {
			return model.data(refreshedIndex, TePictureThumbnailProxyModel::ThumbnailPixmap).canConvert<QPixmap>();
		},
		5000));

	const QVariant thumbnail = model.data(refreshedIndex, TePictureThumbnailProxyModel::ThumbnailPixmap);
	ASSERT_TRUE(thumbnail.canConvert<QPixmap>());
	EXPECT_FALSE(qvariant_cast<QPixmap>(thumbnail).isNull());
}