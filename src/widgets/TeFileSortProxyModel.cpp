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

#include "TeFileSortProxyModel.h"
#include "TeImageLoader.h"

#include <QFileSystemModel>
#include <QImageReader>
#include <QPixmapCache>

TeFileSortProxyModel::TeFileSortProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , m_sortType(TeTypes::ORDER_NAME)
    , m_pixmapSize(128, 128)
    , mp_imageLoader(new TeImageLoader(this))
{
    connect(mp_imageLoader, &TeImageLoader::imageReady,
            this, &TeFileSortProxyModel::onImageReady);
}

TeFileSortProxyModel::~TeFileSortProxyModel()
{
}

void TeFileSortProxyModel::setSortType(TeTypes::OrderType type)
{
    m_sortType = type;
    invalidate();
}

TeTypes::OrderType TeFileSortProxyModel::sortType() const
{
    return m_sortType;
}

void TeFileSortProxyModel::setPixmapSize(const QSize& size)
{
    if (m_pixmapSize == size)
        return;

    m_pixmapSize = size;

    if (rowCount() > 0) {
        emit dataChanged(index(0, 0),
                         index(rowCount() - 1, 0),
                         {FilePixmap});
    }
}

QSize TeFileSortProxyModel::pixmapSize() const
{
    return m_pixmapSize;
}

QVariant TeFileSortProxyModel::data(const QModelIndex& index, int role) const
{
    if (role == FilePixmap) {
        QVariant fileInfoVar = QSortFilterProxyModel::data(index, QFileSystemModel::FileInfoRole);
        if (!fileInfoVar.isValid())
            return QVariant();

        QFileInfo info = qvariant_cast<QFileInfo>(fileInfoVar);
        if (!info.isFile())
            return QVariant();

        if (!QImageReader::supportedImageFormats().contains(info.suffix().toLower().toUtf8()))
            return QVariant();

        QDateTime lastModified = info.lastModified();
        QString key = TeImageLoader::cacheKey(info.absoluteFilePath(), m_pixmapSize, lastModified);
        QPixmap pixmap;
        if (QPixmapCache::find(key, &pixmap)) {
            return pixmap;
        }

        mp_imageLoader->requestLoad(info.absoluteFilePath(), m_pixmapSize);
        return QVariant();
    }

    return QSortFilterProxyModel::data(index, role);
}

void TeFileSortProxyModel::setSourceModel(QAbstractItemModel* model)
{
    if (sourceModel()) {
        if (auto* fsModel = qobject_cast<QFileSystemModel*>(sourceModel())) {
            disconnect(fsModel, &QFileSystemModel::rootPathChanged,
                       this, &TeFileSortProxyModel::onSourceRootPathChanged);
        }
    }

    QSortFilterProxyModel::setSourceModel(model);

    if (auto* fsModel = qobject_cast<QFileSystemModel*>(model)) {
        connect(fsModel, &QFileSystemModel::rootPathChanged,
                this, &TeFileSortProxyModel::onSourceRootPathChanged);
    }
}

void TeFileSortProxyModel::onImageReady(const QString& filePath)
{
    QFileSystemModel* fsModel = qobject_cast<QFileSystemModel*>(sourceModel());
    if (!fsModel)
        return;

    QModelIndex sourceIndex = fsModel->index(filePath);
    if (!sourceIndex.isValid())
        return;

    QModelIndex proxyIndex = mapFromSource(sourceIndex);
    if (!proxyIndex.isValid())
        return;

    emit dataChanged(proxyIndex, proxyIndex, {FilePixmap});
}

void TeFileSortProxyModel::onSourceRootPathChanged()
{
    mp_imageLoader->clearPendingRequests();
}

bool TeFileSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, QFileSystemModel::FileInfoRole);
    QVariant rightData = sourceModel()->data(source_right, QFileSystemModel::FileInfoRole);

    if (!leftData.isValid() || !rightData.isValid())
        return QSortFilterProxyModel::lessThan(source_left, source_right);

    QFileInfo leftFileInfo = qvariant_cast<QFileInfo>(leftData);
    QFileInfo rightFileInfo = qvariant_cast<QFileInfo>(rightData);

    if (leftFileInfo.isDir() && !rightFileInfo.isDir())
        return sortOrder() == Qt::AscendingOrder ? true : false; // Directories come before files
    if (!leftFileInfo.isDir() && rightFileInfo.isDir())
        return sortOrder() == Qt::AscendingOrder ? false : true; // Files come after directories
    
    if (leftFileInfo.isDir() && rightFileInfo.isDir()) {
        // Both are directories, sort by name
        if (leftFileInfo.fileName() == "..")
            return sortOrder() == Qt::AscendingOrder ? true : false; // ".." comes first
        if (rightFileInfo.fileName() == "..")
            return sortOrder() == Qt::AscendingOrder ? false : true; // ".." comes first
        if (sortOrder() == Qt::AscendingOrder){
            return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
        } else {
            return stringLessThan(rightFileInfo.fileName(), leftFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
        }
    }

    switch( sortType()) {
    case TeTypes::ORDER_NAME:
        return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
    case TeTypes::ORDER_SIZE:
        if (leftFileInfo.size() == rightFileInfo.size()) {
            // If sizes are the same, sort by name
            return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
        }
        return leftFileInfo.size() < rightFileInfo.size();
    case TeTypes::ORDER_EXTENSION:
        if (leftFileInfo.suffix() == rightFileInfo.suffix()) {
            // If extensions are the same, sort by name
            return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
        }
        return stringLessThan(leftFileInfo.suffix(), rightFileInfo.suffix(), sortCaseSensitivity(), isSortLocaleAware());
    case TeTypes::ORDER_MODIFIED:
        if (leftFileInfo.lastModified() == rightFileInfo.lastModified()) {
            // If modification times are the same, sort by name
            return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
        }
        return leftFileInfo.lastModified() < rightFileInfo.lastModified();
    default:
        return stringLessThan(leftFileInfo.fileName(), rightFileInfo.fileName(), sortCaseSensitivity(), isSortLocaleAware());
    }
}

bool TeFileSortProxyModel::stringLessThan(const QString &left, const QString &right, Qt::CaseSensitivity cs, bool isLocaleAware)
{
    if (isLocaleAware){
        return left.localeAwareCompare(right) < 0;
    }
    else{
        return left.compare(right, cs) < 0;
    }
}
