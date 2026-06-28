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
#include "utils/TeImageLoader.h"
#include "utils/TeFileInfo.h"

#include <QFileSystemModel>
#include <QImageReader>
#include <QPixmapCache>
/**
 * @file TeFileSortProxyModel.cpp
 * @brief Implementation of TeFileSortProxyModel.
 * @ingroup widgets
 */

TeFileSortProxyModel::TeFileSortProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
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

        static const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
        if (!supportedFormats.contains(info.suffix().toLower().toUtf8()))
            return QVariant();

        QDateTime lastModified = info.lastModified();
        QString key = TeImageLoader::cacheKey(info.absoluteFilePath(), m_pixmapSize, lastModified);
        QPixmap pixmap;
        if (QPixmapCache::find(key, &pixmap)) {
            return pixmap;
        }

        mp_imageLoader->requestLoad(info.absoluteFilePath(), m_pixmapSize);
        return QVariant();
    } else if (role == Qt::DecorationRole){
        if(QSortFilterProxyModel::data(index, Qt::DisplayRole).toString() == "..") {
            //folder up icon
            static const QIcon icon(":/TableEngine/folder_up.png");
            if (!icon.isNull()) {
                return icon;
            }
        }
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

void TeFileSortProxyModel::setFileRegex(const QRegularExpression &re)
{
    m_fileRegex = re;
    invalidateFilter();
}

QRegularExpression TeFileSortProxyModel::fileRegex() const
{
    return m_fileRegex;
}

void TeFileSortProxyModel::setShowHiddenFiles(bool show)
{
    if (m_showHiddenFiles == show)
        return;
    m_showHiddenFiles = show;
    invalidateFilter();
}

bool TeFileSortProxyModel::showHiddenFiles() const
{
    return m_showHiddenFiles;
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

bool TeFileSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)){
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        if (!index.isValid())
            return false;

        QVariant var = sourceModel()->data(index, QFileSystemModel::FileInfoRole);
        if (!var.isValid())
            return true;

        QFileInfo fileInfo = var.value<QFileInfo>();
        const QString fileName = fileInfo.fileName();

        // Filter Unix-style hidden files (dot-prefix) cross-platform.
        // QFileInfo::isHidden() is not used because on Windows it checks the
        // Windows hidden attribute rather than the dot-prefix convention.
        if (!m_showHiddenFiles && fileName.startsWith(QLatin1Char('.')) && fileName != QLatin1String("..")
) {
            return false;
        }

        if(m_fileRegex.pattern().isEmpty()){
            return true;
        }

        if (fileInfo.isDir()) {
            return true; // Always include directories
        }else{
            return m_fileRegex.match(fileName).hasMatch();
        }
    }
    return false;
}

bool TeFileSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    EntryAttr left = entryAttr(source_left);
    EntryAttr right = entryAttr(source_right);

    if (left.isDir && !right.isDir)
        return sortOrder() == Qt::AscendingOrder; // Directories come before files
    if (!left.isDir && right.isDir)
        return sortOrder() != Qt::AscendingOrder; // Files come after directories

    if (left.isDir && right.isDir) {
        // Both are directories, sort by name
        if (left.name == "..")
            return sortOrder() == Qt::AscendingOrder; // ".." comes first
        if (right.name == "..")
            return sortOrder() != Qt::AscendingOrder; // ".." comes first
        if (sortOrder() == Qt::AscendingOrder){
            return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
        } else {
            return stringLessThan(right.name, left.name, sortCaseSensitivity(), isSortLocaleAware());
        }
    }

    switch( sortType()) {
    case TeTypes::ORDER_NAME:
        return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
    case TeTypes::ORDER_SIZE:
        if (left.size == right.size) {
            // If sizes are the same, sort by name
            return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
        }
        return left.size < right.size;
    case TeTypes::ORDER_EXTENSION:
        if (left.suffix == right.suffix) {
            // If extensions are the same, sort by name
            return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
        }
        return stringLessThan(left.suffix, right.suffix, sortCaseSensitivity(), isSortLocaleAware());
    case TeTypes::ORDER_MODIFIED:
        if (left.modified == right.modified) {
            // If modification times are the same, sort by name
            return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
        }
        return left.modified < right.modified;
    }
    return stringLessThan(left.name, right.name, sortCaseSensitivity(), isSortLocaleAware());
}

TeFileSortProxyModel::EntryAttr TeFileSortProxyModel::entryAttr(const QModelIndex& srcIndex) const
{
    EntryAttr attr;
    QVariant fileInfoVar = srcIndex.data(QFileSystemModel::FileInfoRole);
    if (fileInfoVar.isValid()) {
        // Filesystem source: preserve exact QFileInfo-based behavior.
        QFileInfo info = qvariant_cast<QFileInfo>(fileInfoVar);
        attr.isDir = info.isDir();
        attr.size = info.size();
        attr.name = info.fileName();
        attr.suffix = info.suffix();
        attr.modified = info.lastModified();
    } else {
        // Generic source (e.g. QStandardItemModel): read TeFileInfo roles.
        const int type = srcIndex.data(TeFileInfo::ROLE_TYPE).toInt();
        attr.isDir = (type == TeFileInfo::EN_DIR || type == TeFileInfo::EN_PARENT);
        attr.size = srcIndex.data(TeFileInfo::ROLE_SIZE).toLongLong();
        attr.modified = srcIndex.data(TeFileInfo::ROLE_DATE).toDateTime();
        attr.name = srcIndex.data(Qt::DisplayRole).toString();
        attr.suffix = QFileInfo(attr.name).suffix();
    }
    return attr;
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
