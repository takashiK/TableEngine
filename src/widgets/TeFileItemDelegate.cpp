#include "TeFileItemDelegate.h"
#include "TeFileSortProxyModel.h"
#include <QApplication>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include <QtMath>

/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

TeFileItemDelegate::TeFileItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

TeFileItemDelegate::~TeFileItemDelegate()
{
}

void TeFileItemDelegate::setInfoFlags(TeTypes::FileInfoFlags flags)
{
    m_infoFlags = flags;
}

TeTypes::FileInfoFlags TeFileItemDelegate::infoFlags() const
{
    return m_infoFlags;
}

void TeFileItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;

    if(index.data(Qt::DisplayRole).toString() == "..") {
        // Special handling for ".." entry to ensure it gets the correct icon
        qDebug() << "Painting '..' entry, ensuring folder up icon is used.";
    }

    initStyleOption(&opt, index);

    const QWidget* widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    if (opt.decorationPosition == QStyleOptionViewItem::Top) {
        // IconMode: custom text wrapping with WrapAtWordBoundaryOrAnywhere
        QVariant fileInfoVar = index.data(TeFileSortProxyModel::FilePixmap);
        if (fileInfoVar.isValid()) {
            QPixmap pixmap = qvariant_cast<QPixmap>(fileInfoVar);
            if (!pixmap.isNull()) {
                opt.icon = QIcon(pixmap);
            }
        }
        paintIconMode(painter, opt, style, widget);
    } else {
        // ListMode: check for QFileSystemModel extended info
        QVariant fileInfoVar = index.data(QFileSystemModel::FileInfoRole);
        if (fileInfoVar.isValid() && m_infoFlags != TeTypes::FILEINFO_NONE) {
            paintListMode(painter, opt, index, style, widget);
        } else {
            // Standard rendering
            style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        }
    }
}

void TeFileItemDelegate::paintIconMode(QPainter* painter, QStyleOptionViewItem& opt, QStyle* style, const QWidget* widget) const
{
    // Save text, draw item without text (icon + background + selection highlight)
    QString text = opt.text;
    opt.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // Restore text and get text rect
    opt.text = text;
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    const int focusMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, widget) + 1;
    textRect.adjust(focusMargin, 0, -focusMargin, 0);

    // Setup text option with wrap mode based on WrapText feature
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    textOption.setWrapMode((opt.features & QStyleOptionViewItem::WrapText)
        ? QTextOption::WrapAtWordBoundaryOrAnywhere
        : QTextOption::NoWrap);

    // Determine text color from palette
    QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
    QPalette::ColorRole role = (opt.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
    painter->setPen(opt.palette.color(cg, role));
    painter->setFont(opt.font);
    painter->drawText(textRect, text, textOption);
}

void TeFileItemDelegate::paintListMode(QPainter* painter, QStyleOptionViewItem& opt, const QModelIndex& index, QStyle* style, const QWidget* widget) const
{
    QFileInfo fileInfo = qvariant_cast<QFileInfo>(index.data(QFileSystemModel::FileInfoRole));

    // Save text, draw item without text (icon + background + selection highlight)
    QString displayText = opt.text;
    opt.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // Restore text and get text rect
    opt.text = displayText;
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    const int focusMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, widget) + 1;
    textRect.adjust(focusMargin, 0, -focusMargin, 0);

    QFontMetrics fm(opt.font);
    int margin = fm.horizontalAdvance(QLatin1Char(' '));

    // Calculate fixed-width areas from the right
    int rightEdge = textRect.right();
    QRect dateTimeRect;
    QRect sizeRect;

    if (m_infoFlags & TeTypes::FILEINFO_MODIFIED) {
        int dtWidth = fm.horizontalAdvance(QLatin1String("0000/00/00 00:00:00")) + margin * 2;
        dateTimeRect = QRect(rightEdge - dtWidth, textRect.top(), dtWidth, textRect.height());
        rightEdge -= dtWidth;
    }

    if (m_infoFlags & TeTypes::FILEINFO_SIZE) {
        int sizeWidth = fm.horizontalAdvance(QLatin1String("0,000MB")) + margin * 2;
        sizeRect = QRect(rightEdge - sizeWidth, textRect.top(), sizeWidth, textRect.height());
        rightEdge -= sizeWidth;
    }

    QRect displayRect(textRect.left(), textRect.top(), rightEdge - textRect.left(), textRect.height());

    // Determine palette role for text color
    QPalette::ColorRole role = (opt.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
    bool enabled = opt.state & QStyle::State_Enabled;

    // Draw DisplayRole (left-aligned, elided)
    QString elidedText = fm.elidedText(displayText, Qt::ElideRight, displayRect.width());
    style->drawItemText(painter, displayRect, Qt::AlignLeft | Qt::AlignVCenter, opt.palette, enabled, elidedText, role);

    // Draw FileSize (right-aligned within fixed-width area)
    if (m_infoFlags & TeTypes::FILEINFO_SIZE) {
        if (fileInfo.isDir()) {
            style->drawItemText(painter, sizeRect, Qt::AlignRight | Qt::AlignVCenter, opt.palette, enabled, QString(), role);
        } else {
            QString sizeText = formatFileSize(fileInfo.size());
            style->drawItemText(painter, sizeRect, Qt::AlignRight | Qt::AlignVCenter, opt.palette, enabled, sizeText, role);
        }
    }

    // Draw FileModifiedDateTime (right-aligned within fixed-width area)
    if (m_infoFlags & TeTypes::FILEINFO_MODIFIED) {
        QString dtText = fileInfo.lastModified().toString(QLatin1String("yyyy/MM/dd hh:mm:ss"));
        style->drawItemText(painter, dateTimeRect, Qt::AlignRight | Qt::AlignVCenter, opt.palette, enabled, dtText, role);
    }
}

QString TeFileItemDelegate::formatFileSize(qint64 bytes)
{
    if (bytes < 0)
        bytes = 0;

    static const char* units[] = { " B", " KB", " MB", " GB" };
    int unitIndex = 0;
    double value = static_cast<double>(bytes);

    // Scale up units until value fits in 0-9999 range (or reach GB)
    while (unitIndex < 3) {
        if (qRound64(value) < 10000)
            break;
        value /= 1024.0;
        unitIndex++;
    }

    qint64 rounded = qRound64(value);

    // Format with comma thousands separator
    QString numStr;
    if (rounded >= 1000) {
        numStr = QString("%1,%2").arg(rounded / 1000).arg(rounded % 1000, 3, 10, QLatin1Char('0'));
    } else {
        numStr = QString::number(rounded);
    }

    return numStr + QLatin1String(units[unitIndex]);
}

QSize TeFileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.decorationPosition == QStyleOptionViewItem::Top) {
        // Calculate base size without text (icon + margins), same pattern as paintIconMode
        QString text = opt.text;
        opt.text = QString();
        const QWidget* widget = opt.widget;
        QStyle* style = widget ? widget->style() : QApplication::style();
        QSize baseSize = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
        opt.text = text;

        // Independently calculate text size using our wrap mode
        int textWidth = baseSize.width();
        const int focusMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, widget) + 1;
        textWidth -= focusMargin * 2;

        if (textWidth > 0 && !text.isEmpty()) {
            QTextOption textOption;
            textOption.setWrapMode((opt.features & QStyleOptionViewItem::WrapText)
                ? QTextOption::WrapAtWordBoundaryOrAnywhere
                : QTextOption::NoWrap);
            QTextLayout layout(text, opt.font);
            layout.setTextOption(textOption);
            qreal h = 0;
            qreal w = 0;
            layout.beginLayout();
            while (true) {
                QTextLine line = layout.createLine();
                if (!line.isValid()) break;
                line.setLineWidth(textWidth);
                h += line.height();
                w = qMax(w, line.naturalTextWidth());
            }
            layout.endLayout();

            w += focusMargin * 2; // Add horizontal margins back

            baseSize.setWidth(qMax(baseSize.width(), qCeil(w)));
            baseSize.setHeight(baseSize.height() + qCeil(h));
        }
        return baseSize;
    }

    // ListMode: add width for extra fields if applicable
    QSize baseSize = QStyledItemDelegate::sizeHint(option, index);

    QVariant fileInfoVar = index.data(QFileSystemModel::FileInfoRole);
    if (fileInfoVar.isValid() && m_infoFlags != TeTypes::FILEINFO_NONE) {
        QFontMetrics fm(opt.font);
        int margin = fm.horizontalAdvance(QLatin1Char(' '));

        if (m_infoFlags & TeTypes::FILEINFO_SIZE) {
            baseSize.setWidth(baseSize.width() + fm.horizontalAdvance(QLatin1String("0,000 MB")) + margin * 2);
        }
        if (m_infoFlags & TeTypes::FILEINFO_MODIFIED) {
            baseSize.setWidth(baseSize.width() + fm.horizontalAdvance(QLatin1String("0000/00/00 00:00:00")) + margin * 2);
        }
    }

    return baseSize;
}

