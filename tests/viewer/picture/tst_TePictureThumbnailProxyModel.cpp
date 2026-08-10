#include <gmock/gmock.h>
#include <QFileSystemModel>
#include <QImage>
#include <QPixmapCache>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "utils/TeImageLoader.h"
#include "viewer/picture/TePictureThumbnailProxyModel.h"
#include "viewer/picture/TeThumbnailPixmapCache.h"

class tst_TePictureThumbnailProxyModel : public ::testing::Test
{
protected:
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

	QTemporaryDir m_directory;
};

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

TEST_F(tst_TePictureThumbnailProxyModel, decoration_loads_thumbnail_asynchronously)
{
	ASSERT_TRUE(m_directory.isValid());
	const QString path = writeImage("thumbnail.png");
	QPixmapCache::clear();

	QFileSystemModel sourceModel;
	const QModelIndex rootIndex = sourceModel.setRootPath(m_directory.path());
	ASSERT_TRUE(sourceModel.index(path).isValid());

	TePictureThumbnailProxyModel model;
	model.setSourceModel(&sourceModel);
	const QModelIndex index = model.mapFromSource(sourceModel.index(path));
	ASSERT_TRUE(index.isValid());

	QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);
	EXPECT_TRUE(model.data(index, Qt::DecorationRole).canConvert<QIcon>());
	ASSERT_TRUE(dataChangedSpy.wait(5000));

	const QVariant thumbnail = model.data(index, Qt::DecorationRole);
	ASSERT_TRUE(thumbnail.canConvert<QPixmap>());
	EXPECT_FALSE(qvariant_cast<QPixmap>(thumbnail).isNull());

	QPixmap fileListThumbnail;
	EXPECT_FALSE(QPixmapCache::find(
		TeImageLoader::cacheKey(path, QSize(96, 96), QFileInfo(path).lastModified()),
		&fileListThumbnail));
}