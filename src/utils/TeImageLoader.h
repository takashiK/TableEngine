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

#include <QObject>
#include <QSet>
#include <QSize>
#include <QDateTime>
#include <QImage>
#include <QRunnable>

/**
 * @file TeImageLoader.h
 * @brief Asynchronous image loader using QThreadPool.
 * @ingroup utility
 */

/**
 * @class TeImageLoader
 * @brief QObject that manages asynchronous image loading via QThreadPool.
 * @ingroup utility
 *
 * @details requestLoad() posts a TeImageLoadTask to QThreadPool::globalInstance().
 * When the task finishes, onImageLoaded() is invoked via a queued connection and
 * emits imageReady() so that the model can update its decoration role.
 *
 * A set of pending paths (m_pendingRequests) prevents duplicate tasks for the
 * same file.  clearPendingRequests() cancels all outstanding requests.
 *
 * cacheKey() produces a unique string for the (path, size, mtime) triple that
 * can be used as a QPixmapCache key.
 *
 * @see TeFileSortProxyModel, TeImageLoadTask
 */
class TeImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit TeImageLoader(QObject* parent = nullptr);
    ~TeImageLoader();

    /**
     * @brief Queues an asynchronous load for the image at @p filePath.
     *
     * If a load is already pending for @p filePath it is ignored.
     * @param filePath Absolute path to the image file.
     * @param maxSize  Maximum bounding size for the decoded image.
     */
    void requestLoad(const QString& filePath, const QSize& maxSize);

    /**
     * @brief Computes a QPixmapCache key for the given parameters.
     * @param filePath     Absolute file path.
     * @param size         Thumbnail size.
     * @param lastModified File modification time.
     * @return A unique string key.
     */
    static QString cacheKey(const QString& filePath, const QSize& size,
                            const QDateTime& lastModified);

    /** @brief Discards all pending load requests. */
    void clearPendingRequests();

signals:
    /**
     * @brief Emitted after a requested image has been decoded and cached.
     * @param filePath Absolute path of the image that is now ready.
     */
    void imageReady(const QString& filePath);

private slots:
    /** @brief Receives the decoded image from the worker task and emits imageReady(). */
    void onImageLoaded(const QString& filePath, const QSize& size,
                       const QDateTime& lastModified, const QImage& image);

private:
    QSet<QString> m_pendingRequests; ///< Set of file paths currently being loaded.
};

/**
 * @class TeImageLoadTask
 * @brief QRunnable that decodes a single image on a thread-pool thread.
 * @ingroup utility
 *
 * @details Invoked by TeImageLoader::requestLoad().  Reads and scales the image
 * to @c m_maxSize, then invokes TeImageLoader::onImageLoaded() via a
 * queued connection.
 */
class TeImageLoadTask : public QRunnable
{
public:
    /**
     * @brief Constructs a task for the given file and size.
     * @param filePath Absolute image file path.
     * @param maxSize  Maximum bounding size to scale to.
     * @param loader   The TeImageLoader that will receive the result.
     */
    TeImageLoadTask(const QString& filePath, const QSize& maxSize,
                    TeImageLoader* loader);

    /** @brief Loads and scales the image, then signals the loader. */
    void run() override;

private:
    QString        m_filePath;  ///< Absolute path to the image file.
    QSize          m_maxSize;   ///< Maximum bounding size.
    TeImageLoader* mp_loader;   ///< Loader to notify when done.
};
