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
#include <QDateTime>
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

    /**
     * @brief Controls visibility of Unix-style hidden files (names starting with ".").
     * @param show true to show dot-files, false to hide them.
     *
     * @note Detection is based solely on the leading '.' in the filename and is
     *       therefore cross-platform.  QFileInfo::isHidden() is intentionally
     *       avoided because on Windows it tests the Windows "hidden" attribute,
     *       not the dot-prefix convention.
     */
    void setShowHiddenFiles(bool show);
    /** @brief Returns true if dot-files are currently visible. */
    bool showHiddenFiles() const;

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
    /**
     * @brief Source-model-agnostic attributes of a single entry.
     *
     * Used by lessThan() so that ordering works regardless of whether the
     * source model is a QFileSystemModel or a QStandardItemModel.
     */
    struct EntryAttr {
        bool isDir = false;   ///< True if the entry is a directory or "..".
        qint64 size = 0;      ///< File size in bytes.
        QString name;         ///< Display name.
        QString suffix;       ///< File name suffix/extension.
        QDateTime modified;   ///< Last-modified timestamp.
    };

    /**
     * @brief Extracts entry attributes from a source-model index.
     * @param srcIndex Index in the source model.
     * @return Populated EntryAttr describing the entry.
     *
     * @details When QFileSystemModel::FileInfoRole is available the attributes
     * are taken from the QFileInfo; otherwise they are read from the
     * TeFileInfo item roles (ROLE_TYPE / ROLE_SIZE / ROLE_DATE / DisplayRole).
     */
    EntryAttr entryAttr(const QModelIndex& srcIndex) const;

    TeTypes::OrderType m_sortType = TeTypes::ORDER_NAME;        ///< Active sort criterion.
    QSize m_pixmapSize = QSize(128, 128);                   ///< Target thumbnail size.
    TeImageLoader* mp_imageLoader = nullptr;        ///< Async thumbnail loader.
    QRegularExpression m_fileRegex;       ///< Optional filename filter.
    bool m_showHiddenFiles = true;        ///< Whether dot-files are visible.
};
