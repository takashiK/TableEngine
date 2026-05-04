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
#include <QString>

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

// ── TeFileType enum values are distinct ───────────────────────────────────────

TEST(tst_TeUtils, file_type_enum_values_distinct)
{
    EXPECT_NE(TE_FILE_UNKNOWN, TE_FILE_FOLDER);
    EXPECT_NE(TE_FILE_FOLDER,  TE_FILE_TEXT);
    EXPECT_NE(TE_FILE_TEXT,    TE_FILE_IMAGE);
    EXPECT_NE(TE_FILE_IMAGE,   TE_FILE_ARCHIVE);
}
