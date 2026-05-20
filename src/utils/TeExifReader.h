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

#include <QMap>
#include <QString>

/**
 * @file TeExifReader.h
 * @brief Abstract interface for reading EXIF / image metadata.
 * @ingroup utility
 */

/**
 * @class TeExifReader
 * @brief Strategy interface for extracting EXIF and image metadata from a file.
 * @ingroup utility
 *
 * @details Concrete implementations can use QImageReader, a third-party library
 * such as exiv2, or any other source.  The returned map uses human-readable
 * English key names as defined by each implementation.
 *
 * @see TeQImageExifReader
 */
class TeExifReader
{
public:
    virtual ~TeExifReader() = default;

    /**
     * @brief Reads metadata from the file at @p path.
     * @param path Absolute file path.
     * @return Key-value map of metadata entries (English keys, localised or
     *         raw values).  Empty map if the file has no parseable metadata.
     */
    virtual QMap<QString, QString> read(const QString& path) const = 0;
};
