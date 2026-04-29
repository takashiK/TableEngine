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

#include "TeImageLoader.h"

#include <QImageReader>
#include <QFileInfo>
#include <QPixmap>
#include <QPixmapCache>
#include <QThreadPool>
#include <QMetaObject>

TeImageLoader::TeImageLoader(QObject* parent)
    : QObject(parent)
{
}

TeImageLoader::~TeImageLoader()
{
}

void TeImageLoader::requestLoad(const QString& filePath, const QSize& maxSize)
{
    QString pendingKey = QStringLiteral("%1|%2x%3").arg(filePath).arg(maxSize.width()).arg(maxSize.height());
    if (m_pendingRequests.contains(pendingKey))
        return;

    m_pendingRequests.insert(pendingKey);

    TeImageLoadTask* task = new TeImageLoadTask(filePath, maxSize, this);
    task->setAutoDelete(true);
    QThreadPool::globalInstance()->start(task);
}

QString TeImageLoader::cacheKey(const QString& filePath, const QSize& size,
                                const QDateTime& lastModified)
{
    return QStringLiteral("TeImg:%1x%2:%3:%4")
        .arg(size.width())
        .arg(size.height())
        .arg(lastModified.toMSecsSinceEpoch())
        .arg(filePath);
}

void TeImageLoader::clearPendingRequests()
{
    m_pendingRequests.clear();
}

void TeImageLoader::onImageLoaded(const QString& filePath, const QSize& size,
                                  const QDateTime& lastModified, const QImage& image)
{
    QString pendingKey = QStringLiteral("%1|%2x%3").arg(filePath).arg(size.width()).arg(size.height());

    if (!m_pendingRequests.contains(pendingKey))
        return;

    m_pendingRequests.remove(pendingKey);

    if (!image.isNull()) {
        QString cacheKey = TeImageLoader::cacheKey(filePath, size, lastModified);
        QPixmap pixmap = QPixmap::fromImage(image);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    emit imageReady(filePath);
}

TeImageLoadTask::TeImageLoadTask(const QString& filePath, const QSize& maxSize,
                                 TeImageLoader* loader)
    : m_filePath(filePath)
    , m_maxSize(maxSize)
    , mp_loader(loader)
{
}

void TeImageLoadTask::run()
{
    QByteArray format = QImageReader::imageFormat(m_filePath);
    if (format.isEmpty())
        return;

    QImageReader reader(m_filePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();

    if (image.isNull())
        return;

    if (image.width() > m_maxSize.width() || image.height() > m_maxSize.height()) {
        image = image.scaled(m_maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QFileInfo fileInfo(m_filePath);
    QDateTime lastModified = fileInfo.lastModified();

    QMetaObject::invokeMethod(mp_loader, "onImageLoaded", Qt::QueuedConnection,
                              Q_ARG(QString, m_filePath),
                              Q_ARG(QSize, m_maxSize),
                              Q_ARG(QDateTime, lastModified),
                              Q_ARG(QImage, image));
}
