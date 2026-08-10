#include "TeThumbnailPixmapCache.h"

#include <QImage>

#include <algorithm>
#include <limits>

namespace
{
constexpr qint64 BytesPerKiB = 1024;
constexpr qint64 FixedItemOverheadBytes = 256;
}

TeThumbnailPixmapCache::TeThumbnailPixmapCache(qint64 budgetBytes)
	: m_budgetBytes(std::max<qint64>(0, budgetBytes))
{
	const qint64 maxCost = std::min<qint64>(m_budgetBytes / BytesPerKiB, std::numeric_limits<int>::max());
	m_cache.setMaxCost(static_cast<int>(maxCost));
}

const QPixmap* TeThumbnailPixmapCache::find(const QString& key)
{
	return m_cache.object(key);
}

bool TeThumbnailPixmapCache::insert(const QString& key, const QPixmap& pixmap, const QImage& decodedImage)
{
	const qint64 bytes = estimatedBytes(pixmap, decodedImage);
	const int cost = costKiB(bytes);
	if (bytes > m_budgetBytes || cost > m_cache.maxCost()) {
		m_cache.remove(key);
		return false;
	}

	return m_cache.insert(key, new QPixmap(pixmap), cost);
}

void TeThumbnailPixmapCache::clear()
{
	m_cache.clear();
}

qint64 TeThumbnailPixmapCache::budgetBytes() const
{
	return m_budgetBytes;
}

qint64 TeThumbnailPixmapCache::usedBytes() const
{
	return static_cast<qint64>(m_cache.totalCost()) * BytesPerKiB;
}

qint64 TeThumbnailPixmapCache::estimatedBytes(const QPixmap& pixmap, const QImage& decodedImage)
{
	const qint64 depth = std::max(32, pixmap.depth());
	const qint64 pixmapBytes = static_cast<qint64>(pixmap.width()) * pixmap.height() * depth / 8;
	const qint64 decodedBytes = decodedImage.sizeInBytes();
	const qint64 baseBytes = pixmapBytes + decodedBytes;
	return (baseBytes * 5 + 3) / 4 + FixedItemOverheadBytes;
}

int TeThumbnailPixmapCache::costKiB(qint64 bytes)
{
	const qint64 cost = (bytes + BytesPerKiB - 1) / BytesPerKiB;
	return static_cast<int>(std::min<qint64>(cost, std::numeric_limits<int>::max()));
}