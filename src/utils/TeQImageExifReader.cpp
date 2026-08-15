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

#include "TeQImageExifReader.h"

#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QStringList>

#include <limits>

/**
 * @file TeQImageExifReader.cpp
 * @brief Implementation of TeQImageExifReader.
 * @ingroup utility
 */

// ---------------------------------------------------------------------------
// Internal JPEG / TIFF EXIF binary parser
// ---------------------------------------------------------------------------
namespace {

// TIFF type codes
enum TiffType : quint16 {
    TT_BYTE      = 1,
    TT_ASCII     = 2,
    TT_SHORT     = 3,
    TT_LONG      = 4,
    TT_RATIONAL  = 5,
    TT_SBYTE     = 6,
    TT_SSHORT    = 8,
    TT_SLONG     = 9,
    TT_SRATIONAL = 10,
};

// EXIF IFD0 tag IDs
enum ExifTag0 : quint16 {
    TAG_MAKE         = 0x010F,
    TAG_MODEL        = 0x0110,
    TAG_ORIENTATION  = 0x0112,
    TAG_DATETIME     = 0x0132,
    TAG_EXIF_IFD_PTR = 0x8769,
};

// EXIF Sub-IFD tag IDs
enum ExifTagSub : quint16 {
    TAG_EXPOSURE_TIME      = 0x829A,
    TAG_F_NUMBER           = 0x829D,
    TAG_ISO                = 0x8827,
    TAG_DATETIME_ORIGINAL  = 0x9003,
    TAG_FOCAL_LENGTH       = 0x920A,
    TAG_PIXEL_X_DIM        = 0xA002,
    TAG_PIXEL_Y_DIM        = 0xA003,
};

static quint32 tiffTypeSize(quint16 type)
{
    switch (type) {
    case TT_BYTE:  case TT_ASCII: case TT_SBYTE:  return 1;
    case TT_SHORT: case TT_SSHORT:                return 2;
    case TT_LONG:  case TT_SLONG:                 return 4;
    case TT_RATIONAL: case TT_SRATIONAL:          return 8;
    default:                                      return 1;
    }
}

// ---------------------------------------------------------------------------
// Parsing context — all offsets are relative to the TIFF header start
// ---------------------------------------------------------------------------
struct ExifCtx
{
    const QByteArray& data;
    quint32  tiffBase;   // absolute byte offset of the TIFF "II"/"MM" marker
    bool     le;         // true = little-endian (II), false = big-endian (MM)

    bool     boundsOk(qint64 relOff, qint64 len) const
    {
        return relOff >= 0 && len >= 0 && tiffBase <= quint32(data.size()) &&
               relOff <= data.size() - tiffBase - len;
    }

    quint16 u16(qint64 relOff) const
    {
        if (!boundsOk(relOff, 2)) return 0;
        const auto* p = reinterpret_cast<const uchar*>(data.constData()) + tiffBase + relOff;
        return le ? (quint16(p[0]) | (quint16(p[1]) << 8))
                  : ((quint16(p[0]) << 8) | quint16(p[1]));
    }

    quint32 u32(qint64 relOff) const
    {
        if (!boundsOk(relOff, 4)) return 0;
        const auto* p = reinterpret_cast<const uchar*>(data.constData()) + tiffBase + relOff;
        return le ? (quint32(p[0]) | (quint32(p[1])<<8) | (quint32(p[2])<<16) | (quint32(p[3])<<24))
                  : ((quint32(p[0])<<24) | (quint32(p[1])<<16) | (quint32(p[2])<<8) | quint32(p[3]));
    }

    // Read a null-terminated ASCII string stored at relOff with byte length `count`.
    QString ascii(qint64 relOff, quint32 count) const
    {
        if (count == 0 || !boundsOk(relOff, count)) return {};
        const char* s = data.constData() + tiffBase + relOff;
        // Strip trailing NUL and whitespace
        return QString::fromLatin1(s, (int)count - 1).trimmed();
    }

    // Format a RATIONAL (num/den) stored at relOff as a fraction string.
    QString rational(qint64 relOff) const
    {
        if (!boundsOk(relOff, 8)) return {};
        quint32 num = u32(relOff);
        quint32 den = u32(relOff + 4);
        if (den == 0) return {};
        if (num % den == 0)
            return QString::number(num / den);
        return QStringLiteral("%1/%2").arg(num).arg(den);
    }

    void parseIFD(qint64 ifdOff, QMap<QString,QString>& out, bool subIfd) const;
};

// Converts "YYYY:MM:DD HH:MM:SS" → "YYYY/MM/DD HH:MM:SS"
static QString normalizeDateTime(const QString& s)
{
    if (s.size() >= 10 && s[4] == QLatin1Char(':'))
        return s.left(4) + QLatin1Char('/') + s.mid(5, 2) + QLatin1Char('/') + s.mid(8);
    return s;
}

static QString fmtExposure(const QString& frac)
{
    return frac.contains(QLatin1Char('/')) ? (frac + QStringLiteral(" sec"))
                                           : (frac + QStringLiteral(" sec"));
}

static QString fmtFocalLength(const QString& frac)
{
    if (frac.contains(QLatin1Char('/'))) {
        const QStringList p = frac.split(QLatin1Char('/'));
        if (p.size() == 2) {
            double v = p[0].toDouble() / p[1].toDouble();
            return QString::number(v, 'f', 0) + QStringLiteral(" mm");
        }
    }
    return frac + QStringLiteral(" mm");
}

static QString fmtFNumber(const QString& frac)
{
    if (frac.contains(QLatin1Char('/'))) {
        const QStringList p = frac.split(QLatin1Char('/'));
        if (p.size() == 2) {
            double v = p[0].toDouble() / p[1].toDouble();
            return QStringLiteral("F") + QString::number(v, 'f', 1);
        }
    }
    return QStringLiteral("F") + frac;
}

void ExifCtx::parseIFD(qint64 ifdOff, QMap<QString,QString>& out, bool subIfd) const
{
    if (!boundsOk(ifdOff, 2)) return;
    quint16 n = u16(ifdOff);
    if (n > 512) return;  // sanity guard

    qint64 base = ifdOff + 2;
    for (quint16 i = 0; i < n; i++) {
        qint64 e = base + qint64(i) * 12;
        if (!boundsOk(e, 12)) break;

        quint16 tag   = u16(e + 0);
        quint16 type  = u16(e + 2);
        quint32 count = u32(e + 4);

        // Determine where the actual data lives (inline vs. offset)
        qint64 valSize = qint64(count) * tiffTypeSize(type);
        // If value fits in 4 bytes it is stored inline in the value field;
        // otherwise the 4-byte field is an offset from the TIFF header.
        qint64 dataOff = (valSize <= 4) ? (e + 8) : u32(e + 8);

        if (!subIfd) {
            switch (tag) {
            case TAG_MAKE:
                if (type == TT_ASCII)
                    out[QStringLiteral("Make")] = ascii(dataOff, count);
                break;
            case TAG_MODEL:
                if (type == TT_ASCII)
                    out[QStringLiteral("Model")] = ascii(dataOff, count);
                break;
            case TAG_ORIENTATION:
                if (type == TT_SHORT)
                    out[QStringLiteral("Orientation")] = QString::number(u16(dataOff));
                break;
            case TAG_DATETIME:
                if (type == TT_ASCII)
                    out[QStringLiteral("DateTime")] = normalizeDateTime(ascii(dataOff, count));
                break;
            case TAG_EXIF_IFD_PTR:
                // LONG value IS the sub-IFD offset; dataOff points to the inline value
                if (type == TT_LONG)
                    parseIFD(u32(dataOff), out, true);
                break;
            default:
                break;
            }
        } else {
            switch (tag) {
            case TAG_EXPOSURE_TIME:
                if (type == TT_RATIONAL)
                    out[QStringLiteral("ExposureTime")] = fmtExposure(rational(dataOff));
                break;
            case TAG_F_NUMBER:
                if (type == TT_RATIONAL)
                    out[QStringLiteral("FNumber")] = fmtFNumber(rational(dataOff));
                break;
            case TAG_ISO:
                if (type == TT_SHORT)
                    out[QStringLiteral("ISO")] = QString::number(u16(dataOff));
                break;
            case TAG_DATETIME_ORIGINAL:
                if (type == TT_ASCII)
                    out[QStringLiteral("DateTimeOriginal")] =
                        normalizeDateTime(ascii(dataOff, count));
                break;
            case TAG_FOCAL_LENGTH:
                if (type == TT_RATIONAL)
                    out[QStringLiteral("FocalLength")] = fmtFocalLength(rational(dataOff));
                break;
            case TAG_PIXEL_X_DIM:
                if (type == TT_SHORT)
                    out[QStringLiteral("PixelXDimension")] = QString::number(u16(dataOff));
                else if (type == TT_LONG)
                    out[QStringLiteral("PixelXDimension")] = QString::number(u32(dataOff));
                break;
            case TAG_PIXEL_Y_DIM:
                if (type == TT_SHORT)
                    out[QStringLiteral("PixelYDimension")] = QString::number(u16(dataOff));
                else if (type == TT_LONG)
                    out[QStringLiteral("PixelYDimension")] = QString::number(u32(dataOff));
                break;
            default:
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Parse the EXIF APP1 block out of a JPEG byte buffer.
// ---------------------------------------------------------------------------
struct Segment
{
    quint8 marker = 0;
    qint64 payloadOffset = 0;
    qint64 payloadLength = 0;
};

static bool readAt(QFile& file, qint64 offset, char* data, qint64 length)
{
    return offset >= 0 && length >= 0 && file.seek(offset) && file.read(data, length) == length;
}

static bool readU16Be(QFile& file, qint64 offset, quint16* value)
{
    char bytes[2];
    if (!readAt(file, offset, bytes, sizeof(bytes)))
        return false;
    *value = (quint16(uchar(bytes[0])) << 8) | quint16(uchar(bytes[1]));
    return true;
}

static bool readSegment(QFile& file, qint64* position, Segment* segment)
{
    const qint64 fileSize = file.size();
    if (*position < 0 || *position > fileSize - 2)
        return false;
    char value = 0;
    if (!readAt(file, *position, &value, 1) || uchar(value) != 0xff)
        return false;
    qint64 cursor = *position + 1;
    do {
        if (cursor >= fileSize || !readAt(file, cursor++, &value, 1))
            return false;
    } while (uchar(value) == 0xff);
    segment->marker = uchar(value);
    if (segment->marker == 0x00 || segment->marker == 0xd8 ||
        segment->marker == 0xd9 || (segment->marker >= 0xd0 && segment->marker <= 0xd7)) {
        return false;
    }
    quint16 length = 0;
    if (!readU16Be(file, cursor, &length) || length < 2)
        return false;
    segment->payloadOffset = cursor + 2;
    segment->payloadLength = length - 2;
    if (segment->payloadOffset > fileSize - segment->payloadLength)
        return false;
    *position = segment->payloadOffset + segment->payloadLength;
    return true;
}

static bool isSof(quint8 marker)
{
    return (marker >= 0xc0 && marker <= 0xc3) || (marker >= 0xc5 && marker <= 0xc7) ||
           (marker >= 0xc9 && marker <= 0xcb) || (marker >= 0xcd && marker <= 0xcf);
}

struct JpegHeaders
{
    QSize size;
    qint64 entropyOffset = -1;
    QVector<QPair<qint64, QByteArray>> exif;
    QVector<QPair<qint64, QByteArray>> mpf;
};

static bool scanHeaders(QFile& file, qint64 start, JpegHeaders* headers)
{
    char soi[2];
    if (!readAt(file, start, soi, sizeof(soi)) || uchar(soi[0]) != 0xff || uchar(soi[1]) != 0xd8)
        return false;
    qint64 position = start + 2;
    while (true) {
        Segment segment;
        if (!readSegment(file, &position, &segment))
            return false;
        if (isSof(segment.marker) && segment.payloadLength >= 5) {
            char sof[5];
            if (!readAt(file, segment.payloadOffset, sof, sizeof(sof)))
                return false;
            headers->size = QSize((quint16(uchar(sof[3])) << 8) | uchar(sof[4]),
                                  (quint16(uchar(sof[1])) << 8) | uchar(sof[2]));
        }
        if ((segment.marker == 0xe1 || segment.marker == 0xe2) && segment.payloadLength >= 4) {
            if (segment.payloadLength > 65533)
                return false;
            QByteArray payload(segment.payloadLength, Qt::Uninitialized);
            if (!readAt(file, segment.payloadOffset, payload.data(), payload.size()))
                return false;
            if (segment.marker == 0xe1 && payload.startsWith("Exif\0\0"))
                headers->exif.append(qMakePair(segment.payloadOffset, payload));
            if (segment.marker == 0xe2 && payload.startsWith("MPF\0"))
                headers->mpf.append(qMakePair(segment.payloadOffset, payload));
        }
        if (segment.marker == 0xda) {
            headers->entropyOffset = position;
            return true;
        }
    }
}

static qint64 findJpegEnd(QFile& file, qint64 entropyOffset, qint64 scanEnd)
{
    qint64 position = entropyOffset;
    const qint64 fileSize = file.size();
    const qint64 limit = scanEnd > 0 && scanEnd < fileSize ? scanEnd : fileSize;
    constexpr qint64 BufferSize = 64 * 1024;
    bool markerPending = false;
    qint64 markerStart = -1;
    while (position < limit) {
        if (!file.seek(position))
            return -1;
        const QByteArray buffer = file.read(qMin(BufferSize, limit - position));
        if (buffer.isEmpty())
            return -1;
        for (int index = 0; index < buffer.size(); ++index, ++position) {
            const quint8 byte = uchar(buffer.at(index));
            if (!markerPending) {
                if (byte == 0xff) {
                    markerPending = true;
                    markerStart = position;
                }
                continue;
            }
            if (byte == 0xff)
                continue;

            markerPending = false;
            if (byte == 0x00 || (byte >= 0xd0 && byte <= 0xd7))
                continue;
            if (byte == 0xd9)
                return position + 1;
            if (byte == 0xd8)
                return -1;

            position = markerStart;
            Segment segment;
            if (!readSegment(file, &position, &segment))
                return -1;
            break;
        }
    }
    return -1;
}

struct TiffView
{
    const QByteArray& data;
    qint64 base = 0;
    bool littleEndian = false;

    bool contains(qint64 offset, qint64 length) const
    {
        return offset >= 0 && length >= 0 && base >= 0 && base <= data.size() &&
               offset <= data.size() - base - length;
    }

    bool u16(qint64 offset, quint16* value) const
    {
        if (!contains(offset, 2)) return false;
        const auto* p = reinterpret_cast<const uchar*>(data.constData() + base + offset);
        *value = littleEndian ? quint16(p[0]) | (quint16(p[1]) << 8)
                              : (quint16(p[0]) << 8) | quint16(p[1]);
        return true;
    }

    bool u32(qint64 offset, quint32* value) const
    {
        if (!contains(offset, 4)) return false;
        const auto* p = reinterpret_cast<const uchar*>(data.constData() + base + offset);
        *value = littleEndian ? quint32(p[0]) | (quint32(p[1]) << 8) |
                                     (quint32(p[2]) << 16) | (quint32(p[3]) << 24)
                              : (quint32(p[0]) << 24) | (quint32(p[1]) << 16) |
                                     (quint32(p[2]) << 8) | quint32(p[3]);
        return true;
    }
};

static qint64 tiffValueSize(quint16 type, quint32 count)
{
    qint64 unit = 0;
    switch (type) {
    case 1: case 2: case 6: case 7: unit = 1; break;
    case 3: case 8: unit = 2; break;
    case 4: case 9: case 11: unit = 4; break;
    case 5: case 10: case 12: unit = 8; break;
    default: return -1;
    }
    return count <= std::numeric_limits<qint64>::max() / unit ? count * unit : -1;
}

static bool tiffEntry(const TiffView& view, qint64 ifd, quint16 wantedTag,
                      quint16* type, quint32* count, qint64* valueOffset)
{
    quint16 entries = 0;
    if (!view.u16(ifd, &entries) || entries > 512 || ifd > std::numeric_limits<qint64>::max() - 2)
        return false;
    const qint64 first = ifd + 2;
    if (entries > (view.data.size() - view.base - first) / 12)
        return false;
    for (quint16 index = 0; index < entries; ++index) {
        const qint64 entry = first + qint64(index) * 12;
        quint16 tag = 0;
        quint16 entryType = 0;
        quint32 entryCount = 0;
        if (!view.u16(entry, &tag) || !view.u16(entry + 2, &entryType) ||
            !view.u32(entry + 4, &entryCount))
            return false;
        const qint64 size = tiffValueSize(entryType, entryCount);
        if (size < 0)
            continue;
        quint32 offset = 0;
        if (!view.u32(entry + 8, &offset))
            return false;
        const qint64 actualOffset = size <= 4 ? entry + 8 : offset;
        if (!view.contains(actualOffset, size))
            continue;
        if (tag == wantedTag) {
            *type = entryType;
            *count = entryCount;
            *valueOffset = actualOffset;
            return true;
        }
    }
    return false;
}

static bool tiffNextIfd(const TiffView& view, qint64 ifd, quint32* next)
{
    quint16 entries = 0;
    if (!view.u16(ifd, &entries) || entries > 512 ||
        ifd > std::numeric_limits<qint64>::max() - 2 ||
        entries > (view.data.size() - view.base - (ifd + 2)) / 12)
        return false;
    return view.u32(ifd + 2 + qint64(entries) * 12, next);
}

static bool makeTiffView(const QByteArray& payload, qint64 base, TiffView* view, quint32* ifd0)
{
    if (base < 0 || base > payload.size() - 8)
        return false;
    if (&view->data != &payload)
        return false;
    const bool little = payload[base] == 'I' && payload[base + 1] == 'I';
    const bool big = payload[base] == 'M' && payload[base + 1] == 'M';
    if (!little && !big)
        return false;
    view->base = base;
    view->littleEndian = little;
    quint16 magic = 0;
    return view->u16(2, &magic) && magic == 42 && view->u32(4, ifd0);
}

struct ExifThumbnail
{
    qint64 offset = -1;
    qint64 length = -1;
    int primaryOrientation = 0;
    int thumbnailOrientation = 0;
};

static ExifThumbnail parseExifThumbnail(const QByteArray& payload)
{
    ExifThumbnail thumbnail;
    TiffView view{payload, 0, false};
    quint32 ifd0 = 0;
    if (!makeTiffView(payload, 6, &view, &ifd0))
        return thumbnail;
    quint16 type = 0;
    quint32 count = 0;
    qint64 valueOffset = 0;
    if (tiffEntry(view, ifd0, 0x0112, &type, &count, &valueOffset) && type == 3 && count == 1) {
        quint16 orientation = 0;
        if (view.u16(valueOffset, &orientation))
            thumbnail.primaryOrientation = orientation;
    }
    quint32 ifd1 = 0;
    if (!tiffNextIfd(view, ifd0, &ifd1) || ifd1 == 0)
        return thumbnail;
    if (tiffEntry(view, ifd1, 0x0112, &type, &count, &valueOffset) && type == 3 && count == 1) {
        quint16 orientation = 0;
        if (view.u16(valueOffset, &orientation))
            thumbnail.thumbnailOrientation = orientation;
    }
    quint32 offset = 0;
    quint32 length = 0;
    if (tiffEntry(view, ifd1, 0x0201, &type, &count, &valueOffset) && type == 4 && count == 1)
        view.u32(valueOffset, &offset);
    if (tiffEntry(view, ifd1, 0x0202, &type, &count, &valueOffset) && type == 4 && count == 1)
        view.u32(valueOffset, &length);
    if (offset != 0 && length != 0 && view.contains(offset, length)) {
        thumbnail.offset = offset;
        thumbnail.length = length;
    }
    return thumbnail;
}

struct MpfEntry
{
    quint32 attributes = 0;
    quint32 size = 0;
    quint32 offset = 0;
};

static QVector<MpfEntry> parseMpf(const QByteArray& payload)
{
    QVector<MpfEntry> entries;
    TiffView view{payload, 0, false};
    quint32 ifd0 = 0;
    if (!makeTiffView(payload, 4, &view, &ifd0))
        return entries;
    quint16 type = 0;
    quint32 count = 0;
    qint64 valueOffset = 0;
    quint32 imageCount = 0;
    if (!tiffEntry(view, ifd0, 0xb001, &type, &count, &valueOffset) ||
        type != 4 || count != 1 || !view.u32(valueOffset, &imageCount) || imageCount == 0 ||
        imageCount > 128)
        return entries;
    if (!tiffEntry(view, ifd0, 0xb002, &type, &count, &valueOffset) ||
        count != imageCount * 16u || !view.contains(valueOffset, count))
        return entries;
    for (quint32 index = 0; index < imageCount; ++index) {
        const qint64 entry = valueOffset + qint64(index) * 16;
        MpfEntry image;
        if (!view.u32(entry, &image.attributes) || !view.u32(entry + 4, &image.size) ||
            !view.u32(entry + 8, &image.offset))
            return {};
        entries.append(image);
    }
    return entries;
}

static QMap<QString, QString> parseExifMetadata(const QByteArray& payload)
{
    QMap<QString, QString> result;
    if (payload.size() < 14 || !payload.startsWith("Exif\0\0"))
        return result;
    const bool little = payload[6] == 'I' && payload[7] == 'I';
    const bool big = payload[6] == 'M' && payload[7] == 'M';
    if (!little && !big)
        return result;
    ExifCtx context{payload, 6, little};
    if (context.u16(2) == 0x002a)
        context.parseIFD(context.u32(4), result, false);
    return result;
}

} // namespace

// ---------------------------------------------------------------------------
// TeQImageExifReader::read
// ---------------------------------------------------------------------------
QMap<QString,QString> TeQImageExifReader::read(const QString& path) const
{
    QMap<QString,QString> result;

    // ── 1. Image dimensions via QImageReader (no full decode) ──────────────
    QImageReader reader(path);
    const QSize sz = reader.size();
    if (sz.isValid()) {
        result[QStringLiteral("Width")]  = QString::number(sz.width());
        result[QStringLiteral("Height")] = QString::number(sz.height());
    }

    // ── 2. Qt text metadata (format-specific, e.g. PNG tEXt chunks) ────────
    for (const QString& key : reader.textKeys()) {
        const QString val = reader.text(key).trimmed();
        if (!val.isEmpty())
            result[key] = val;
    }

    // ── 3. JPEG binary EXIF parser ─────────────────────────────────────────
    const QByteArray fmt = QImageReader::imageFormat(path).toLower();
    if (fmt == "jpeg" || fmt == "jpg") {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            JpegHeaders headers;
            scanHeaders(file, 0, &headers);
            for (const auto& app1 : headers.exif) {
                const QMap<QString, QString> exif = parseExifMetadata(app1.second);
                for (auto it = exif.cbegin(); it != exif.cend(); ++it)
                    result[it.key()] = it.value();
            }
        }
    }

    return result;
}

TeEmbeddedImageSet TeQImageExifReader::scanImages(const QString& path) const
{
    TeEmbeddedImageSet result;
    const QFileInfo fileInfo(path);
    if (!fileInfo.isFile() || fileInfo.size() < 4)
        return result;

    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return result;
    JpegHeaders primary;
    if (!scanHeaders(file, 0, &primary))
        return result;
    const qint64 primaryEnd = findJpegEnd(file, primary.entropyOffset, 0);
    if (primaryEnd <= 0)
        return result;

    result.m_path = path;
    result.m_fileSize = fileInfo.size();
    result.m_lastModified = fileInfo.lastModified();
    auto appendImage = [&](qint64 offset, qint64 length, const QSize& encodedSize,
                           TeEmbeddedImageKind kind, TeEmbeddedImageOrigin origin,
                           const QString& detail, int orientation) {
        if (offset < 0 || length <= 0 || offset > result.m_fileSize - length)
            return;
        for (int index = 0; index < result.m_ranges.size(); ++index) {
            if (result.m_ranges[index] == qMakePair(offset, length))
                return;
        }
        TeEmbeddedImageInfo info;
        info.id = quint32(result.m_images.size());
        info.encodedSize = encodedSize;
        info.byteSize = length;
        info.format = QStringLiteral("JPEG");
        info.kind = kind;
        info.origin = origin;
        info.originDetail = detail;
        info.orientation = orientation;
        result.m_ranges.append(qMakePair(offset, length));
        result.m_images.append(info);
    };
    int primaryOrientation = 0;
    for (const auto& app1 : primary.exif) {
        const ExifThumbnail thumbnail = parseExifThumbnail(app1.second);
        if (thumbnail.primaryOrientation != 0) {
            primaryOrientation = thumbnail.primaryOrientation;
            break;
        }
    }
    appendImage(0, primaryEnd, primary.size, TeEmbeddedImageKind::Primary,
                TeEmbeddedImageOrigin::JpegPrimary, QStringLiteral("SOI"), primaryOrientation);

    for (const auto& app1 : primary.exif) {
        const ExifThumbnail thumbnail = parseExifThumbnail(app1.second);
        if (thumbnail.offset < 0 || thumbnail.length < 0 ||
            thumbnail.offset > result.m_fileSize - thumbnail.length)
            continue;
        const qint64 offset = app1.first + 6 + thumbnail.offset;
        if (offset < 0 || offset > result.m_fileSize - thumbnail.length)
            continue;
        JpegHeaders embedded;
        if (!scanHeaders(file, offset, &embedded))
            continue;
        const qint64 end = findJpegEnd(file, embedded.entropyOffset,
                           offset + thumbnail.length);
        if (end < offset || end - offset > thumbnail.length)
            continue;
        appendImage(offset, end - offset, embedded.size, TeEmbeddedImageKind::Thumbnail,
                    TeEmbeddedImageOrigin::ExifIfd1, QStringLiteral("IFD1"),
                    thumbnail.thumbnailOrientation);
    }

    for (const auto& app2 : primary.mpf) {
        const QVector<MpfEntry> entries = parseMpf(app2.second);
        for (int index = 0; index < entries.size(); ++index) {
            const MpfEntry& entry = entries[index];
            const qint64 offset = index == 0 ? 0 : app2.first + 4 + qint64(entry.offset);
            const qint64 declaredLength = entry.size;
            if (declaredLength <= 0 || offset < 0 || offset > result.m_fileSize - declaredLength)
                continue;
            JpegHeaders embedded;
            if (!scanHeaders(file, offset, &embedded))
                continue;
            const qint64 end = findJpegEnd(file, embedded.entropyOffset,
                                           offset + declaredLength);
            if (end < offset || end - offset > declaredLength)
                continue;
            int mpfOrientation = 0;
            for (const auto& app1 : embedded.exif) {
                const ExifThumbnail info = parseExifThumbnail(app1.second);
                if (info.primaryOrientation != 0) {
                    mpfOrientation = info.primaryOrientation;
                    break;
                }
            }
            const quint32 mpType = entry.attributes & 0x00ffffffu;
            const TeEmbeddedImageKind kind = index == 0 ? TeEmbeddedImageKind::Primary :
                (mpType == 0x010001u || mpType == 0x010002u ? TeEmbeddedImageKind::Preview :
                                                             TeEmbeddedImageKind::Auxiliary);
            appendImage(offset, end - offset, embedded.size, kind, TeEmbeddedImageOrigin::Mpf,
                        QStringLiteral("MPF image %1").arg(index), mpfOrientation);
        }
    }
    return result;
}
