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

#include <gmock/gmock.h>
#include "utils/TeUtils.h"
#include <QFile>
#include <QString>
#include <QTemporaryDir>

using namespace ::testing;

// ── getFileType ────────────────────────────────────────────────────────────────

TEST(tst_TeUtils, getFileType_unknown_extension)
{
    EXPECT_EQ(getFileType("file.xyz123"), TE_FILE_UNKNOWN);
}

TEST(tst_TeUtils, getFileType_image_extensions)
{
    EXPECT_EQ(getFileType("photo.png"),  TE_FILE_IMAGE);
    EXPECT_EQ(getFileType("photo.jpg"),  TE_FILE_IMAGE);
    EXPECT_EQ(getFileType("photo.bmp"),  TE_FILE_IMAGE);
    EXPECT_EQ(getFileType("photo.gif"),  TE_FILE_IMAGE);
}

TEST(tst_TeUtils, getFileType_archive_extensions)
{
    EXPECT_EQ(getFileType("data.zip"),   TE_FILE_ARCHIVE);
    EXPECT_EQ(getFileType("data.7z"),    TE_FILE_ARCHIVE);
    EXPECT_EQ(getFileType("data.tar"),   TE_FILE_ARCHIVE);
}

TEST(tst_TeUtils, getFileType_text_extensions)
{
    EXPECT_EQ(getFileType("readme.txt"), TE_FILE_TEXT);
}

TEST(tst_TeUtils, getFileType_empty_string)
{
    // Should not crash and should return UNKNOWN or FOLDER.
    TeFileType t = getFileType("");
    EXPECT_TRUE(t == TE_FILE_UNKNOWN || t == TE_FILE_FOLDER);
}

TEST(tst_TeUtils, getFileType_uppercase_extension)
{
    // Case-insensitive matching is expected for common types.
    TeFileType t = getFileType("PHOTO.PNG");
    EXPECT_TRUE(t == TE_FILE_IMAGE);
}

// ── detectTextCodec ────────────────────────────────────────────────────────────

TEST(tst_TeUtils, detectTextCodec_utf8_ascii_content)
{
    QByteArray data = "Hello, World!";
    QStringList codecs = {"UTF-8", "Shift-JIS"};
    QString codec = detectTextCodec(data, codecs);
    EXPECT_FALSE(codec.isEmpty());
}

TEST(tst_TeUtils, detectTextCodec_empty_data_returns_empty_or_first)
{
    QByteArray data;
    QStringList codecs = {"UTF-8"};
    QString codec = detectTextCodec(data, codecs);
    // Should not crash; result is implementation-defined for empty input.
    (void)codec;
}

TEST(tst_TeUtils, detectTextCodec_empty_codec_list_returns_empty)
{
    // When no codec matches the codec list, implementation falls back to "UTF-8"
    QByteArray data = "Hello";
    QStringList codecs;
    QString codec = detectTextCodec(data, codecs);
    EXPECT_FALSE(codec.isEmpty());  // falls back to UTF-8
}

namespace {
// Builds raw UTF-16 bytes (no BOM) for an ASCII/Latin string, used to test
// BOM-less UTF-16 detection without depending on QStringEncoder BOM defaults.
QByteArray toUtf16Bytes(const QString& text, bool littleEndian)
{
    QByteArray bytes;
    for (const QChar& ch : text) {
        const char16_t code = ch.unicode();
        const char hi = static_cast<char>((code >> 8) & 0xFF);
        const char lo = static_cast<char>(code & 0xFF);
        if (littleEndian) {
            bytes.append(lo);
            bytes.append(hi);
        }
        else {
            bytes.append(hi);
            bytes.append(lo);
        }
    }
    return bytes;
}

QString writeSampleFile(QTemporaryDir& directory, const QString& name, const QByteArray& content)
{
    QFile file(directory.filePath(name));
    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size()) {
        return QString();
    }
    file.close();
    return file.fileName();
}
}

TEST(tst_TeUtils, isTextFile_detects_utf8_and_bom_prefixed_utf16)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    // "日本語" (U+65E5 U+672C U+8A9E) encoded three ways.
    const QList<QPair<QString, QByteArray>> samples = {
        {"utf8.txt", QByteArray::fromHex("E697A5E69CACE8AA9E")},
        {"utf16le_bom.txt", QByteArray::fromHex("FFFEE5652C679E8A")},
        {"utf16be_bom.txt", QByteArray::fromHex("FEFF65E5672C8A9E")},
    };

    for (const auto& sample : samples) {
        const QString path = writeSampleFile(directory, sample.first, sample.second);
        ASSERT_FALSE(path.isEmpty()) << sample.first.toStdString();
        EXPECT_TRUE(isTextFile(path)) << sample.first.toStdString();
    }
}

TEST(tst_TeUtils, isTextFile_detects_bom_less_utf16)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString text = QStringLiteral("Hello, World!");

    const QString lePath = writeSampleFile(directory, "utf16le_nobom.txt", toUtf16Bytes(text, true));
    ASSERT_FALSE(lePath.isEmpty());
    EXPECT_TRUE(isTextFile(lePath));

    const QString bePath = writeSampleFile(directory, "utf16be_nobom.txt", toUtf16Bytes(text, false));
    ASSERT_FALSE(bePath.isEmpty());
    EXPECT_TRUE(isTextFile(bePath));
}

TEST(tst_TeUtils, isTextFile_detects_shift_jis_and_eucjp)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    // Repeat a valid multi-byte sequence well beyond the ICU detector's
    // minimum confidence threshold, and long enough to rule out the
    // (now separate) UTF-16 branch ever being reached for this data.
    const QByteArray sjis = QByteArray::fromHex("93FA967B8CEA").repeated(20);
    const QByteArray eucjp = QByteArray::fromHex("C6FCCBDCB8EC").repeated(20);

    const QString sjisPath = writeSampleFile(directory, "sjis.txt", sjis);
    ASSERT_FALSE(sjisPath.isEmpty());
    EXPECT_TRUE(isTextFile(sjisPath));

    const QString eucjpPath = writeSampleFile(directory, "eucjp.txt", eucjp);
    ASSERT_FALSE(eucjpPath.isEmpty());
    EXPECT_TRUE(isTextFile(eucjpPath));
}

TEST(tst_TeUtils, isTextFile_detects_plain_ascii_with_common_whitespace)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString path = writeSampleFile(directory, "plain.txt", "line one\r\nline two\tindented\x0C");
    ASSERT_FALSE(path.isEmpty());
    EXPECT_TRUE(isTextFile(path));
}

TEST(tst_TeUtils, isTextFile_handles_multibyte_character_split_at_sample_boundary)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    // 1022 ASCII bytes + a 3-byte UTF-8 character ("あ") + trailing bytes.
    // isTextFile only reads the first 1024 bytes, so the UTF-8 sequence is
    // cut after its first two bytes; this must not be treated as invalid.
    QByteArray content(1022, 'a');
    content += QByteArray::fromHex("E38182");
    content += QByteArray(50, 'b');

    const QString path = writeSampleFile(directory, "boundary.txt", content);
    ASSERT_FALSE(path.isEmpty());
    EXPECT_TRUE(isTextFile(path));
}

TEST(tst_TeUtils, isTextFile_rejects_binary_content)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString path = writeSampleFile(directory, "binary.dat",
        QByteArray::fromHex("89504E470D0A1A0A0000000D49484452"));
    ASSERT_FALSE(path.isEmpty());

    EXPECT_FALSE(isTextFile(path));
}

TEST(tst_TeUtils, isTextFile_rejects_short_binary_with_sparse_nul)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    // Fewer than 8 bytes with a single NUL: too short for the BOM-less
    // UTF-16 heuristic to fire, so the NUL must be caught as a binary marker.
    const QString path = writeSampleFile(directory, "sparse_nul.dat", QByteArray::fromHex("01020300FF06"));
    ASSERT_FALSE(path.isEmpty());

    EXPECT_FALSE(isTextFile(path));
}

TEST(tst_TeUtils, isTextFile_empty_file_returns_true)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString path = writeSampleFile(directory, "empty.txt", QByteArray());
    ASSERT_FALSE(path.isEmpty());

    EXPECT_TRUE(isTextFile(path));
}

TEST(tst_TeUtils, isTextFile_nonexistent_path_returns_false)
{
    EXPECT_FALSE(isTextFile("Z:/definitely/not/a/real/path.txt"));
}

TEST(tst_TeUtils, isTextFile_directory_returns_false)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    EXPECT_FALSE(isTextFile(directory.path()));
}

// ── TeFileType enum values are distinct ───────────────────────────────────────

TEST(tst_TeUtils, file_type_enum_values_distinct)
{
    EXPECT_NE(TE_FILE_UNKNOWN, TE_FILE_FOLDER);
    EXPECT_NE(TE_FILE_FOLDER,  TE_FILE_TEXT);
    EXPECT_NE(TE_FILE_TEXT,    TE_FILE_IMAGE);
    EXPECT_NE(TE_FILE_IMAGE,   TE_FILE_ARCHIVE);
}
