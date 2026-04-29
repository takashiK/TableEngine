#pragma once

#include <QStyledItemDelegate>
#include "TeTypes.h"

/**
 * @file TeFileItemDelegate.h
 * @brief Item delegate for rendering file entries in TeFileListView.
 * @ingroup widgets
 */

/**
 * @class TeFileItemDelegate
 * @brief QStyledItemDelegate subclass that draws file items with icons,
 *        thumbnails, and optional attribute text.
 * @ingroup widgets
 *
 * @details Provides custom painting for the two view modes:
 * - **Icon mode** - large thumbnail with filename below.
 * - **List/detail mode** - smaller icon with filename plus optional columns
 *   (size, date, attributes) depending on the active FileInfoFlags.
 *
 * The delegate falls back to the default QStyle rendering for state
 * decorations (selection highlight, focus rect).
 *
 * @see TeFileListView, TeFileSortProxyModel, TeTypes::FileInfoFlags
 */
class TeFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TeFileItemDelegate(QObject* parent = nullptr);
    ~TeFileItemDelegate() override;

    /**
     * @brief Sets which file attribute columns are visible in detail mode.
     * @param flags Bitmask of TeTypes::FileInfoFlags values.
     */
    void setInfoFlags(TeTypes::FileInfoFlags flags);
    /** @brief Returns the active file-info display flags. */
    TeTypes::FileInfoFlags infoFlags() const;

    /**
     * @brief Renders an item using the icon or list-mode layout.
     * @param painter The painter to draw with.
     * @param option  Style options including the view's current state.
     * @param index   The model index of the item to render.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * @brief Returns the preferred size for an item cell.
     * @param option Style options.
     * @param index  The model index of the item.
     * @return Minimum size needed to display the item without clipping.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    /** @brief Renders the item in icon-mode layout (large icon + name below). */
    void paintIconMode(QPainter* painter, QStyleOptionViewItem& opt, QStyle* style, const QWidget* widget) const;

    /** @brief Renders the item in list/detail-mode layout. */
    void paintListMode(QPainter* painter, QStyleOptionViewItem& opt, const QModelIndex& index, QStyle* style, const QWidget* widget) const;

    /**
     * @brief Converts a byte count to a human-readable string (e.g. "1.2 MB").
     * @param bytes Raw file size in bytes.
     * @return Formatted size string.
     */
    static QString formatFileSize(qint64 bytes);

    TeTypes::FileInfoFlags m_infoFlags = TeTypes::FILEINFO_NONE; ///< Active attribute display flags.
};