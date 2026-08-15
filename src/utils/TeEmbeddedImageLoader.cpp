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

#include "TeEmbeddedImageLoader.h"

#include "TeQImageExifReader.h"

#include <QImageReader>
#include <QTransform>

#include <cmath>

namespace
{
bool swapsDimensions(int orientation)
{
    return orientation >= 5 && orientation <= 8;
}

int validOrientation(int orientation)
{
    return orientation >= 1 && orientation <= 8 ? orientation : 1;
}

int orientationFromTransformation(QImageIOHandler::Transformations transformation)
{
    switch (transformation) {
    case QImageIOHandler::TransformationMirror: return 2;
    case QImageIOHandler::TransformationFlip: return 4;
    case QImageIOHandler::TransformationRotate180: return 3;
    case QImageIOHandler::TransformationRotate90: return 6;
    case QImageIOHandler::TransformationMirrorAndRotate90: return 5;
    case QImageIOHandler::TransformationFlipAndRotate90: return 7;
    case QImageIOHandler::TransformationRotate270: return 8;
    default: return 1;
    }
}

QTransform orientationTransform(int orientation)
{
    switch (validOrientation(orientation)) {
    case 2: return QTransform().scale(-1, 1);
    case 3: return QTransform().rotate(180);
    case 4: return QTransform().scale(1, -1);
    case 5: return QTransform().rotate(90) * QTransform().scale(-1, 1);
    case 6: return QTransform().rotate(90);
    case 7: return QTransform().rotate(90) * QTransform().scale(1, -1);
    case 8: return QTransform().rotate(270);
    default: return QTransform();
    }
}

QSize rawDecodeTarget(const QSize& targetSize, int sourceOrientation)
{
    if (!targetSize.isValid() || targetSize.width() <= 0 || targetSize.height() <= 0)
        return {};
    return swapsDimensions(sourceOrientation) ? targetSize.transposed() : targetSize;
}

QImage decodeImage(QImageReader& reader, const QSize& targetSize, int sourceOrientation)
{
    reader.setAutoTransform(false);
    const QSize scaledSize = rawDecodeTarget(targetSize, sourceOrientation);
    const QSize imageSize = reader.size();
    if (scaledSize.isValid() && imageSize.isValid() &&
        (imageSize.width() > scaledSize.width() || imageSize.height() > scaledSize.height())) {
        reader.setScaledSize(imageSize.scaled(scaledSize, Qt::KeepAspectRatio));
    }
    return reader.read();
}

QImage decodeDirect(const QString& path, const QSize& targetSize, int fallbackOrientation,
                    int* sourceOrientation)
{
    QImageReader reader(path);
    const QImageIOHandler::Transformations transformation = reader.transformation();
    const int orientation = transformation == QImageIOHandler::TransformationNone
        ? fallbackOrientation
        : orientationFromTransformation(transformation);
    if (sourceOrientation)
        *sourceOrientation = validOrientation(orientation);
    return decodeImage(reader, targetSize, orientation);
}

QImage normalizeOrientation(const QImage& image, int sourceOrientation, int outputOrientation)
{
    bool invertible = false;
    const QTransform outputInverse = orientationTransform(outputOrientation).inverted(&invertible);
    if (!invertible)
        return image;
    return image.transformed(orientationTransform(sourceOrientation) * outputInverse);
}

QImage scaleImage(QImage image, const QSize& targetSize, int outputOrientation)
{
    if (!targetSize.isValid() || targetSize.width() <= 0 || targetSize.height() <= 0)
        return image;
    QSize rawTarget = targetSize;
    if (swapsDimensions(outputOrientation))
        rawTarget.transpose();
    if (image.width() > rawTarget.width() || image.height() > rawTarget.height())
        image = image.scaled(rawTarget, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return image;
}
} // namespace

QSize teEmbeddedImageDisplaySize(const QSize& size, int orientation)
{
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0)
        return {};
    return swapsDimensions(orientation) ? size.transposed() : size;
}

TeEmbeddedImageSelection teSelectEmbeddedImage(const QVector<TeEmbeddedImageInfo>& images,
                                                const QSize& targetSize)
{
    TeEmbeddedImageSelection selection;
    const TeEmbeddedImageInfo* primary = nullptr;
    for (const TeEmbeddedImageInfo& image : images) {
        if (image.kind == TeEmbeddedImageKind::Primary ||
            image.origin == TeEmbeddedImageOrigin::JpegPrimary) {
            primary = &image;
            break;
        }
    }
    if (!primary)
        return selection;

    selection.image = *primary;
    selection.primaryOrientation = validOrientation(primary->orientation);
    const QSize primarySize = teEmbeddedImageDisplaySize(primary->encodedSize,
                                                         selection.primaryOrientation);
    if (!targetSize.isValid() || !primarySize.isValid())
        return selection;

    const QSize fittedPrimary = primarySize.scaled(targetSize, Qt::KeepAspectRatio);
    const double primaryAspect = double(primarySize.width()) / primarySize.height();
    const TeEmbeddedImageInfo* best = nullptr;
    qint64 bestPixels = 0;
    for (const TeEmbeddedImageInfo& image : images) {
        if (image.kind == TeEmbeddedImageKind::Primary ||
            image.origin == TeEmbeddedImageOrigin::JpegPrimary)
            continue;
        const int orientation = image.orientation == 0 ? selection.primaryOrientation : image.orientation;
        const QSize candidateSize = teEmbeddedImageDisplaySize(image.encodedSize, orientation);
        if (!candidateSize.isValid() || candidateSize.width() < fittedPrimary.width() ||
            candidateSize.height() < fittedPrimary.height())
            continue;
        const double candidateAspect = double(candidateSize.width()) / candidateSize.height();
        if (std::abs(candidateAspect - primaryAspect) > primaryAspect * 0.01)
            continue;
        const qint64 pixels = qint64(image.encodedSize.width()) * image.encodedSize.height();
        if (!best || pixels < bestPixels) {
            best = &image;
            bestPixels = pixels;
        }
    }
    if (best) {
        selection.image = *best;
        selection.useEmbeddedImage = true;
    }
    return selection;
}

TeEmbeddedImageSelection teSelectEmbeddedImage(const TeEmbeddedImageSet& images,
                                                const QSize& targetSize)
{
    return teSelectEmbeddedImage(images.images(), targetSize);
}

QImage teDecodeEmbeddedImage(const QString& path, const TeEmbeddedImageSet& images,
                             const TeEmbeddedImageSelection& selection,
                             const QSize& targetSize, int outputOrientation,
                             bool allowDirectFallback)
{
    const int primaryOrientation = validOrientation(selection.primaryOrientation);
    const int targetOrientation = validOrientation(outputOrientation);
    QImage image;
    int sourceOrientation = primaryOrientation;
    if (selection.useEmbeddedImage) {
        std::unique_ptr<QIODevice> device = images.openImageDevice(selection.image.id);
        if (device && device->open(QIODevice::ReadOnly)) {
            QImageReader reader(device.get(), selection.image.format.toLatin1());
            sourceOrientation = validOrientation(selection.image.orientation == 0
                                                 ? primaryOrientation : selection.image.orientation);
            image = decodeImage(reader, targetSize, sourceOrientation);
        }
    }
    if (image.isNull() && allowDirectFallback) {
        image = decodeDirect(path, targetSize, primaryOrientation, &sourceOrientation);
    }
    if (image.isNull())
        return image;
    return scaleImage(normalizeOrientation(image, sourceOrientation, targetOrientation),
                      targetSize, targetOrientation);
}

QImage teDecodeEmbeddedImage(const QString& path, const QSize& targetSize,
                             int outputOrientation, TeEmbeddedImageSelection* selection)
{
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(path);
    const TeEmbeddedImageSelection selected = teSelectEmbeddedImage(images, targetSize);
    if (selection)
        *selection = selected;
    return teDecodeEmbeddedImage(path, images, selected, targetSize, outputOrientation);
}