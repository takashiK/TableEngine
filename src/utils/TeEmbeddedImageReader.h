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

#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QPair>
#include <QSize>
#include <QString>
#include <QVector>

#include <memory>

/**
 * @file TeEmbeddedImageReader.h
 * @brief Safe, lazy access to encoded images embedded in JPEG files.
 * @ingroup utility
 */

enum class TeEmbeddedImageKind {
    Primary,
    Thumbnail,
    Preview,
    Auxiliary
};

enum class TeEmbeddedImageOrigin {
    JpegPrimary,
    ExifIfd1,
    Mpf,
    MakerNote
};

struct TeEmbeddedImageInfo
{
    quint32 id = 0;
    QSize encodedSize;
    qint64 byteSize = 0;
    QString format;
    TeEmbeddedImageKind kind = TeEmbeddedImageKind::Auxiliary;
    TeEmbeddedImageOrigin origin = TeEmbeddedImageOrigin::JpegPrimary;
    QString originDetail;
    int orientation = 0;
};

class TeFileSliceDevice : public QIODevice
{
public:
    TeFileSliceDevice(const QString& path, qint64 offset, qint64 length,
                      qint64 expectedFileSize, const QDateTime& expectedLastModified);

    bool open(OpenMode mode) override;
    void close() override;
    bool isSequential() const override;
    qint64 size() const override;
    bool seek(qint64 position) override;

protected:
    qint64 readData(char* data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 maxSize) override;

private:
    QString m_path;
    qint64 m_offset = 0;
    qint64 m_length = 0;
    qint64 m_expectedFileSize = 0;
    QDateTime m_expectedLastModified;
    std::unique_ptr<QFile> m_file;
};

class TeEmbeddedImageSet
{
public:
    TeEmbeddedImageSet() = default;

    const QVector<TeEmbeddedImageInfo>& images() const;
    std::unique_ptr<QIODevice> openImageDevice(quint32 id) const;

private:
    friend class TeQImageExifReader;

    QString m_path;
    qint64 m_fileSize = 0;
    QDateTime m_lastModified;
    QVector<TeEmbeddedImageInfo> m_images;
    QVector<QPair<qint64, qint64>> m_ranges;
};

class TeEmbeddedImageReader
{
public:
    virtual ~TeEmbeddedImageReader() = default;
    virtual TeEmbeddedImageSet scanImages(const QString& path) const = 0;
};
