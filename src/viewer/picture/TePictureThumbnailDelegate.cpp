#include "TePictureThumbnailDelegate.h"
#include "TePictureThumbnailProxyModel.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

/**
 * @file TePictureThumbnailDelegate.cpp
 * @brief Implementation of TePictureThumbnailDelegate.
 * @ingroup viewer
 */

namespace
{
constexpr int ItemVerticalMargin = 2;
}

TePictureThumbnailDelegate::TePictureThumbnailDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

TePictureThumbnailDelegate::~TePictureThumbnailDelegate()
{
}

void TePictureThumbnailDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	const QVariant thumbnail = index.data(TePictureThumbnailProxyModel::ThumbnailPixmap);
	if (thumbnail.canConvert<QPixmap>()) {
		const QPixmap pixmap = qvariant_cast<QPixmap>(thumbnail);
		if (!pixmap.isNull()) {
			opt.icon = QIcon(pixmap);
			opt.features |= QStyleOptionViewItem::HasDecoration;
		}
	}

	const QWidget* widget = opt.widget;
	QStyle* style = widget ? widget->style() : QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}

QSize TePictureThumbnailDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// Reserve room for a full sized thumbnail so item geometry never changes while loading.
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(qMax(size.height(), option.decorationSize.height() + 2 * ItemVerticalMargin));
	size.setWidth(qMax(size.width(), option.decorationSize.width() + 2 * ItemVerticalMargin));
	return size;
}
