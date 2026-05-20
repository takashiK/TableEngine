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
#include <QImageReader>
#include <QStringList>

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

    bool     boundsOk(quint32 relOff, quint32 len) const
    {
        return (tiffBase + relOff + len) <= (quint32)data.size();
    }

    quint16 u16(quint32 relOff) const
    {
        if (!boundsOk(relOff, 2)) return 0;
        const auto* p = reinterpret_cast<const uchar*>(data.constData()) + tiffBase + relOff;
        return le ? (quint16(p[0]) | (quint16(p[1]) << 8))
                  : ((quint16(p[0]) << 8) | quint16(p[1]));
    }

    quint32 u32(quint32 relOff) const
    {
        if (!boundsOk(relOff, 4)) return 0;
        const auto* p = reinterpret_cast<const uchar*>(data.constData()) + tiffBase + relOff;
        return le ? (quint32(p[0]) | (quint32(p[1])<<8) | (quint32(p[2])<<16) | (quint32(p[3])<<24))
                  : ((quint32(p[0])<<24) | (quint32(p[1])<<16) | (quint32(p[2])<<8) | quint32(p[3]));
    }

    // Read a null-terminated ASCII string stored at relOff with byte length `count`.
    QString ascii(quint32 relOff, quint32 count) const
    {
        if (count == 0 || !boundsOk(relOff, count)) return {};
        const char* s = data.constData() + tiffBase + relOff;
        // Strip trailing NUL and whitespace
        return QString::fromLatin1(s, (int)count - 1).trimmed();
    }

    // Format a RATIONAL (num/den) stored at relOff as a fraction string.
    QString rational(quint32 relOff) const
    {
        if (!boundsOk(relOff, 8)) return {};
        quint32 num = u32(relOff);
        quint32 den = u32(relOff + 4);
        if (den == 0) return {};
        if (num % den == 0)
            return QString::number(num / den);
        return QStringLiteral("%1/%2").arg(num).arg(den);
    }

    void parseIFD(quint32 ifdOff, QMap<QString,QString>& out, bool subIfd) const;
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

void ExifCtx::parseIFD(quint32 ifdOff, QMap<QString,QString>& out, bool subIfd) const
{
    if (!boundsOk(ifdOff, 2)) return;
    quint16 n = u16(ifdOff);
    if (n > 512) return;  // sanity guard

    quint32 base = ifdOff + 2;
    for (quint16 i = 0; i < n; i++) {
        quint32 e = base + i * 12u;
        if (!boundsOk(e, 12)) break;

        quint16 tag   = u16(e + 0);
        quint16 type  = u16(e + 2);
        quint32 count = u32(e + 4);

        // Determine where the actual data lives (inline vs. offset)
        quint32 valSize   = count * tiffTypeSize(type);
        // If value fits in 4 bytes it is stored inline in the value field;
        // otherwise the 4-byte field is an offset from the TIFF header.
        quint32 dataOff = (valSize <= 4) ? (e + 8) : u32(e + 8);

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
static QMap<QString,QString> parseJpegExif(const QByteArray& data)
{
    QMap<QString,QString> result;
    const int sz = data.size();
    if (sz < 4) return result;

    // Verify SOI
    if ((uchar)data[0] != 0xFF || (uchar)data[1] != 0xD8) return result;

    int pos = 2;
    while (pos + 4 <= sz) {
        if ((uchar)data[pos] != 0xFF) break;
        uchar marker = (uchar)data[pos + 1];
        quint16 segLen = (quint16(uchar(data[pos+2])) << 8) | quint16(uchar(data[pos+3]));
        if (segLen < 2) break;

        if (marker == 0xE1 && pos + 10 <= sz) {
            // Check "Exif\0\0" header
            if (data[pos+4]=='E' && data[pos+5]=='x' && data[pos+6]=='i' &&
                data[pos+7]=='f' && data[pos+8]==0   && data[pos+9]==0)
            {
                quint32 tiffBase = (quint32)(pos + 10);
                if ((int)(tiffBase + 8) > sz) break;

                bool le = ((uchar)data[tiffBase] == 'I' && (uchar)data[tiffBase+1] == 'I');
                ExifCtx ctx{data, tiffBase, le};

                // Verify TIFF magic 0x002A
                if (ctx.u16(2) != 0x002A) break;

                quint32 ifd0Off = ctx.u32(4);
                ctx.parseIFD(ifd0Off, result, false);
            }
            break;  // Only one APP1 segment matters
        }

        // Skip to next segment (marker 2 bytes + length includes the 2 length bytes)
        if (segLen == 0) break;
        pos += 2 + segLen;
    }
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
            // APP1 segment is always within the first 64 KB
            const QByteArray head = file.read(65536);
            file.close();
            const QMap<QString,QString> exif = parseJpegExif(head);
            for (auto it = exif.cbegin(); it != exif.cend(); ++it)
                result[it.key()] = it.value();
        }
    }

    return result;
}
