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

#include "TeEmbeddedImageReader.h"

#include <QFileInfo>

TeFileSliceDevice::TeFileSliceDevice(const QString& path, qint64 offset, qint64 length,
                                     qint64 expectedFileSize,
                                     const QDateTime& expectedLastModified)
    : m_path(path)
    , m_offset(offset)
    , m_length(length)
    , m_expectedFileSize(expectedFileSize)
    , m_expectedLastModified(expectedLastModified)
{
}

bool TeFileSliceDevice::open(OpenMode mode)
{
    if (mode != ReadOnly || m_offset < 0 || m_length < 0 ||
        m_offset > m_expectedFileSize - m_length) {
        return false;
    }

    const QFileInfo info(m_path);
    if (!info.isFile() || info.size() != m_expectedFileSize ||
        info.lastModified() != m_expectedLastModified) {
        return false;
    }

    m_file = std::make_unique<QFile>(m_path);
    if (!m_file->open(QFile::ReadOnly) || !m_file->seek(m_offset)) {
        m_file.reset();
        return false;
    }
    QIODevice::open(mode);
    return true;
}

void TeFileSliceDevice::close()
{
    m_file.reset();
    QIODevice::close();
}

bool TeFileSliceDevice::isSequential() const
{
    return false;
}

qint64 TeFileSliceDevice::size() const
{
    return m_length;
}

bool TeFileSliceDevice::seek(qint64 position)
{
    if (!isOpen() || position < 0 || position > m_length || !m_file)
        return false;
    if (!m_file->seek(m_offset + position))
        return false;
    return QIODevice::seek(position);
}

qint64 TeFileSliceDevice::readData(char* data, qint64 maxSize)
{
    if (!m_file || maxSize <= 0)
        return 0;
    const qint64 remaining = m_length - pos();
    if (remaining <= 0)
        return 0;
    return m_file->read(data, qMin(maxSize, remaining));
}

qint64 TeFileSliceDevice::writeData(const char*, qint64)
{
    return -1;
}

const QVector<TeEmbeddedImageInfo>& TeEmbeddedImageSet::images() const
{
    return m_images;
}

std::unique_ptr<QIODevice> TeEmbeddedImageSet::openImageDevice(quint32 id) const
{
    for (int index = 0; index < m_images.size(); ++index) {
        if (m_images[index].id == id) {
            const auto range = m_ranges[index];
            return std::make_unique<TeFileSliceDevice>(m_path, range.first, range.second,
                                                       m_fileSize, m_lastModified);
        }
    }
    return nullptr;
}