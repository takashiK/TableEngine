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

#include "TeExifReader.h"

/**
 * @file TeQImageExifReader.h
 * @brief TeExifReader implementation backed by QImageReader and a lightweight
 *        JPEG EXIF binary parser.
 * @ingroup utility
 */

/**
 * @class TeQImageExifReader
 * @brief Reads image dimensions and EXIF metadata without external libraries.
 * @ingroup utility
 *
 * @details Combines two strategies:
 * 1. QImageReader::size() and QImageReader::text() for format-agnostic metadata.
 * 2. A hand-written JPEG APP1 / TIFF IFD parser for JPEG EXIF tags:
 *    Make, Model, Orientation, DateTime, ExposureTime, FNumber, ISO,
 *    DateTimeOriginal, FocalLength, PixelXDimension, PixelYDimension.
 *
 * The binary parser reads at most 64 KB from the file head, which covers
 * virtually all JPEG APP1 segments.
 *
 * Keys returned (subset, always English):
 *   "Width", "Height", "Make", "Model", "Orientation", "DateTime",
 *   "DateTimeOriginal", "ExposureTime", "FNumber", "ISO", "FocalLength",
 *   "PixelXDimension", "PixelYDimension"
 *
 * @see TeExifReader
 */
class TeQImageExifReader : public TeExifReader
{
public:
    QMap<QString, QString> read(const QString& path) const override;
};
