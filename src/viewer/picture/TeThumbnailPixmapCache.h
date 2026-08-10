#pragma once

#include <QCache>
#include <QPixmap>
#include <QString>

class QImage;

class TeThumbnailPixmapCache
{
public:
	static constexpr qint64 DefaultBudgetBytes = 64LL * 1024 * 1024;

	explicit TeThumbnailPixmapCache(qint64 budgetBytes = DefaultBudgetBytes);

	const QPixmap* find(const QString& key);
	bool insert(const QString& key, const QPixmap& pixmap, const QImage& decodedImage);
	void clear();

	qint64 budgetBytes() const;
	qint64 usedBytes() const;
	static qint64 estimatedBytes(const QPixmap& pixmap, const QImage& decodedImage);

private:
	static int costKiB(qint64 bytes);

	QCache<QString, QPixmap> m_cache;
	qint64 m_budgetBytes;
};