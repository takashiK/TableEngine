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

#include "TeEmbeddedImageReader.h"

#include <QImage>

struct TeEmbeddedImageSelection
{
    TeEmbeddedImageInfo image;
    int primaryOrientation = 1;
    bool useEmbeddedImage = false;
};

QSize teEmbeddedImageDisplaySize(const QSize& size, int orientation);
TeEmbeddedImageSelection teSelectEmbeddedImage(const QVector<TeEmbeddedImageInfo>& images,
                                                const QSize& targetSize);
TeEmbeddedImageSelection teSelectEmbeddedImage(const TeEmbeddedImageSet& images,
                                                const QSize& targetSize);
QImage teDecodeEmbeddedImage(const QString& path, const TeEmbeddedImageSet& images,
                             const TeEmbeddedImageSelection& selection,
                             const QSize& targetSize, int outputOrientation = 1,
                             bool allowDirectFallback = true);
QImage teDecodeEmbeddedImage(const QString& path, const QSize& targetSize,
                             int outputOrientation = 1,
                             TeEmbeddedImageSelection* selection = nullptr);