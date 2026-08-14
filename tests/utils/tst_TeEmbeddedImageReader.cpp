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

#include <gtest/gtest.h>

#include "utils/TeQImageExifReader.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QTemporaryDir>

namespace {

void put16(QByteArray& data, int offset, quint16 value)
{
    data[offset] = char(value & 0xff);
    data[offset + 1] = char(value >> 8);
}

void put32(QByteArray& data, int offset, quint32 value)
{
    for (int index = 0; index < 4; ++index)
        data[offset + index] = char(value >> (index * 8));
}

QByteArray jpeg(const QSize& size, QRgb color)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(color);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QImageWriter writer(&buffer, "jpeg");
    writer.setQuality(90);
    EXPECT_TRUE(writer.write(image));
    return buffer.data();
}

QByteArray segment(quint8 marker, const QByteArray& payload)
{
    QByteArray result;
    result.append(char(0xff));
    result.append(char(marker));
    result.append(char((payload.size() + 2) >> 8));
    result.append(char((payload.size() + 2) & 0xff));
    result.append(payload);
    return result;
}

QByteArray exifPayload(const QByteArray& thumbnail, quint32 thumbnailOffset = 80)
{
    QByteArray payload(6 + 80, '\0');
    payload.replace(0, 6, QByteArray("Exif\0\0", 6));
    payload[6] = 'I';
    payload[7] = 'I';
    put16(payload, 8, 42);
    put32(payload, 10, 8);
    put16(payload, 14, 1);
    put16(payload, 16, 0x0112);
    put16(payload, 18, 3);
    put32(payload, 20, 1);
    put16(payload, 24, 6);
    put32(payload, 28, 26);
    put16(payload, 32, 4);
    put16(payload, 34, 0x0103);
    put16(payload, 36, 3);
    put32(payload, 38, 1);
    put16(payload, 42, 6);
    put16(payload, 46, 0x0112);
    put16(payload, 48, 3);
    put32(payload, 50, 1);
    put16(payload, 54, 3);
    put16(payload, 58, 0x0201);
    put16(payload, 60, 4);
    put32(payload, 62, 1);
    put32(payload, 66, thumbnailOffset);
    put16(payload, 70, 0x0202);
    put16(payload, 72, 4);
    put32(payload, 74, 1);
    put32(payload, 78, thumbnail.size());
    payload.append(thumbnail);
    return payload;
}

QByteArray orientationExifPayload(quint16 orientation)
{
    QByteArray payload(6 + 26, '\0');
    payload.replace(0, 6, QByteArray("Exif\0\0", 6));
    payload[6] = 'I';
    payload[7] = 'I';
    put16(payload, 8, 42);
    put32(payload, 10, 8);
    put16(payload, 14, 1);
    put16(payload, 16, 0x0112);
    put16(payload, 18, 3);
    put32(payload, 20, 1);
    put16(payload, 24, orientation);
    put32(payload, 28, 0);
    return payload;
}

QByteArray mpfPayload(quint32 primarySize, quint32 previewOffset, quint32 previewSize,
                      quint32 previewType = 0x08010002u)
{
    QByteArray payload(4 + 38 + 32, '\0');
    payload.replace(0, 4, QByteArray("MPF\0", 4));
    payload[4] = 'I';
    payload[5] = 'I';
    put16(payload, 6, 42);
    put32(payload, 8, 8);
    put16(payload, 12, 2);
    put16(payload, 14, 0xb001);
    put16(payload, 16, 4);
    put32(payload, 18, 1);
    put32(payload, 22, 2);
    put16(payload, 26, 0xb002);
    put16(payload, 28, 7);
    put32(payload, 30, 32);
    put32(payload, 34, 38);
    put32(payload, 42, 0x10030000u);
    put32(payload, 46, primarySize);
    put32(payload, 50, 0);
    put32(payload, 58, previewType);
    put32(payload, 62, previewSize);
    put32(payload, 66, previewOffset);
    return payload;
}

QByteArray makeContainer(bool corruptOffset = false, bool externalOffset = false,
                         bool postEntropyMarker = false, quint32 previewType = 0x08010002u)
{
    QByteArray primary = jpeg(QSize(16, 10), qRgb(255, 0, 0));
    if (postEntropyMarker)
        primary = primary.left(primary.size() - 2) + segment(0xfe, QByteArray("marker")) + primary.right(2);
    const QByteArray thumbnail = jpeg(QSize(4, 3), qRgb(0, 255, 0));
    QByteArray preview = jpeg(QSize(8, 5), qRgb(0, 0, 255));
    preview = preview.left(2) + segment(0xe1, orientationExifPayload(8)) + preview.mid(2);
    QByteArray exif = segment(0xe1, exifPayload(thumbnail));
    QByteArray mpf = segment(0xe2, mpfPayload(0, 0, preview.size()));
    const quint32 primarySize = primary.size() + exif.size() + mpf.size();
    const quint32 mpHeaderOffset = 2 + quint32(exif.size()) + 8;
    const quint32 previewOffset = primarySize - mpHeaderOffset;
    mpf = segment(0xe2, mpfPayload(primarySize, previewOffset, preview.size(), previewType));
    if (corruptOffset)
        exif = segment(0xe1, exifPayload(thumbnail, 0xfffffff0u));
    else if (externalOffset)
        exif = segment(0xe1, exifPayload(thumbnail, primarySize - 12));
    return primary.left(2) + exif + mpf + primary.mid(2) + preview;
}

QString writeContainer(QTemporaryDir& directory, bool corruptOffset = false,
                       bool externalOffset = false)
{
    const QString path = directory.filePath(QStringLiteral("images.jpg"));
    QFile file(path);
    EXPECT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(makeContainer(corruptOffset, externalOffset));
    return path;
}

} // namespace

TEST(tst_TeEmbeddedImageReader, scans_and_decodes_generated_embedded_images)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(writeContainer(directory));

    ASSERT_EQ(images.images().size(), 3);
    EXPECT_EQ(images.images()[0].kind, TeEmbeddedImageKind::Primary);
    EXPECT_EQ(images.images()[0].encodedSize, QSize(16, 10));
    EXPECT_EQ(images.images()[0].orientation, 6);
    EXPECT_EQ(images.images()[1].origin, TeEmbeddedImageOrigin::ExifIfd1);
    EXPECT_EQ(images.images()[1].encodedSize, QSize(4, 3));
    EXPECT_EQ(images.images()[1].orientation, 3);
    EXPECT_EQ(images.images()[2].origin, TeEmbeddedImageOrigin::Mpf);
    EXPECT_EQ(images.images()[2].kind, TeEmbeddedImageKind::Preview);
    EXPECT_EQ(images.images()[2].orientation, 8);

    std::unique_ptr<QIODevice> device = images.openImageDevice(images.images()[1].id);
    ASSERT_NE(device, nullptr);
    ASSERT_TRUE(device->open(QIODevice::ReadOnly));
    QImageReader imageReader(device.get(), "jpeg");
    EXPECT_EQ(imageReader.size(), QSize(4, 3));
    EXPECT_FALSE(imageReader.read().isNull());
}

TEST(tst_TeEmbeddedImageReader, slice_device_stays_within_image_range)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(writeContainer(directory));
    ASSERT_FALSE(images.images().isEmpty());
    std::unique_ptr<QIODevice> device = images.openImageDevice(images.images().front().id);
    ASSERT_TRUE(device->open(QIODevice::ReadOnly));
    const qint64 imageSize = device->size();
    EXPECT_EQ(device->readAll().size(), imageSize);
    EXPECT_FALSE(device->seek(imageSize + 1));
    EXPECT_TRUE(device->seek(imageSize));
    EXPECT_TRUE(device->readAll().isEmpty());
    EXPECT_EQ(device->write("x", 1), -1);
}

TEST(tst_TeEmbeddedImageReader, rejects_invalid_ifd1_offset)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(writeContainer(directory, true));
    ASSERT_EQ(images.images().size(), 2);
    EXPECT_EQ(images.images()[0].origin, TeEmbeddedImageOrigin::JpegPrimary);
    EXPECT_EQ(images.images()[1].origin, TeEmbeddedImageOrigin::Mpf);
}

TEST(tst_TeEmbeddedImageReader, rejects_ifd1_offset_outside_app1)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(writeContainer(directory, false, true));
    ASSERT_EQ(images.images().size(), 2);
    EXPECT_EQ(images.images()[0].origin, TeEmbeddedImageOrigin::JpegPrimary);
    EXPECT_EQ(images.images()[1].origin, TeEmbeddedImageOrigin::Mpf);
}

TEST(tst_TeEmbeddedImageReader, scans_jpeg_with_marker_after_entropy_data)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("images.jpg"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(makeContainer(false, false, true));
    file.close();

    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(path);
    ASSERT_EQ(images.images().size(), 3);
    EXPECT_EQ(images.images()[0].encodedSize, QSize(16, 10));
}

TEST(tst_TeEmbeddedImageReader, classifies_vga_large_thumbnail_as_preview)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("images.jpg"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(makeContainer(false, false, false, 0x08010001u));
    file.close();

    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(path);
    ASSERT_EQ(images.images().size(), 3);
    EXPECT_EQ(images.images()[2].kind, TeEmbeddedImageKind::Preview);
}

TEST(tst_TeEmbeddedImageReader, reads_exif_before_an_incomplete_jpeg_header)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("truncated.jpg"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    const QByteArray thumbnail = jpeg(QSize(4, 3), qRgb(0, 255, 0));
    file.write(QByteArray("\xff\xd8", 2) + segment(0xe1, exifPayload(thumbnail)) +
               QByteArray("\xff\xe0\x00\x10", 4));
    file.close();

    TeQImageExifReader reader;
    EXPECT_EQ(reader.read(path).value(QStringLiteral("Orientation")), QStringLiteral("6"));
}

TEST(tst_TeEmbeddedImageReader, rejects_open_when_file_changes_after_scan)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeContainer(directory);
    TeQImageExifReader reader;
    const TeEmbeddedImageSet images = reader.scanImages(path);
    ASSERT_FALSE(images.images().isEmpty());
    std::unique_ptr<QIODevice> device = images.openImageDevice(images.images().front().id);
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::Append));
    file.write("x", 1);
    file.close();
    EXPECT_FALSE(device->open(QIODevice::ReadOnly));
}