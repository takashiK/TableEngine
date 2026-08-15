#pragma once

#include <QStyledItemDelegate>

/**
 * @file TePictureThumbnailDelegate.h
 * @brief Declaration of TePictureThumbnailDelegate.
 * @ingroup viewer
 */

/**
 * @class TePictureThumbnailDelegate
 * @brief Draws the thumbnail published through TePictureThumbnailProxyModel::ThumbnailPixmap.
 *
 * Until a thumbnail is decoded the item keeps the file type icon provided by the
 * source model, so the list stays readable while loading progresses.
 */
class TePictureThumbnailDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit TePictureThumbnailDelegate(QObject* parent = nullptr);
	~TePictureThumbnailDelegate() override;

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
