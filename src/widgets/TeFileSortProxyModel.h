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

#pragma once

#include <QSortFilterProxyModel>
#include <QSize>
#include "TeTypes.h"

/**
 * @file TeFileSortProxyModel.h
 * @brief Sort/filter proxy model with async thumbnail loading for file views.
 * @ingroup widgets
 */

class TeImageLoader;

/**
 * @class TeFileSortProxyModel
 * @brief QSortFilterProxyModel subclass with natural-sort, type-aware ordering,
 *        and asynchronous thumbnail loading.
 * @ingroup widgets
 *
 * @details Sits between a QFileSystemModel and a TeFileListView.  Provides:
 * - Natural-sort string comparison via stringLessThan().
 * - Type-aware ordering where directories always precede files when
 *   @c OrderType::FOLDER_FIRST is in effect.
 * - On-demand thumbnail loading using TeImageLoader.  Pixmaps are cached
 *   inside the source model as Qt::DecorationRole data; when a thumb
 *   becomes available the onImageReady() slot invalidates the relevant row.
 * - Optional filename regex filter via setFileRegex().
 *
 * @see TeImageLoader, TeFileListView
 */
class TeFileSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief Custom model data roles.
     */
    enum Role {
        FilePixmap = Qt::UserRole + 50, ///< Loaded thumbnail pixmap for the item.
    };
    Q_ENUM(Role)

    explicit TeFileSortProxyModel(QObject* parent = nullptr);
    virtual ~TeFileSortProxyModel();

    /**
     * @brief Sets the sort criterion.
     * @param type The column/ordering strategy to use.
     */
    void setSortType(TeTypes::OrderType type);
    /** @brief Returns the current sort criterion. */
    TeTypes::OrderType sortType() const;

    /**
     * @brief Sets the size at which thumbnails are loaded.
     * @param size Pixel size for thumbnail images.
     */
    void setPixmapSize(const QSize& size);
    /** @brief Returns the configured thumbnail pixel size. */
    QSize pixmapSize() const;

    /**
     * @brief Returns model data, loading a thumbnail when @p role is FilePixmap.
     * @param index The model index.
     * @param role  The data role requested.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Connects to the source model and hooks onSourceRootPathChanged().
     * @param sourceModel The underlying QFileSystemModel.
     */
    void setSourceModel(QAbstractItemModel* sourceModel) override;

    /**
     * @brief Sets a filename filter; only files matching @p re are shown.
     * @param re Regular expression applied to the filename.
     */
    void setFileRegex(const QRegularExpression& re);
    /** @brief Returns the current filename filter regex. */
    QRegularExpression fileRegex() const;

protected:
    /**
     * @brief Filters rows based on file type flags and the filename regex.
     * @param source_row    Row index in the source model.
     * @param source_parent Parent index in the source model.
     * @return true if the row should be visible.
     */
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    /**
     * @brief Compares two items using the configured sort type.
     * @param source_left  Left-hand model index.
     * @param source_right Right-hand model index.
     * @return true if @p source_left should appear before @p source_right.
     */
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    /**
     * @brief Natural-sort helper for string comparisons.
     * @param left          Left string.
     * @param right         Right string.
     * @param cs            Case sensitivity.
     * @param isLocaleAware Use locale-aware comparison.
     * @return true if @p left < @p right.
     */
    static bool stringLessThan(const QString &left, const QString &right, Qt::CaseSensitivity cs, bool isLocaleAware);

private slots:
    /**
     * @brief Invalidates the row for @p filePath after its thumbnail is ready.
     * @param filePath Absolute path of the file whose thumb was loaded.
     */
    void onImageReady(const QString& filePath);

    /** @brief Clears the cached thumbnail data when the root path changes. */
    void onSourceRootPathChanged();

private:
    TeTypes::OrderType m_sortType;        ///< Active sort criterion.
    QSize m_pixmapSize;                   ///< Target thumbnail size.
    TeImageLoader* mp_imageLoader;        ///< Async thumbnail loader.
    QRegularExpression m_fileRegex;       ///< Optional filename filter.
};
