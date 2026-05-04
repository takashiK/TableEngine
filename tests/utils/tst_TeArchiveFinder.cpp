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
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "utils/TeArchiveFinder.h"
#include "utils/TeSearchQuery.h"
#include "utils/TeArchive.h"
#include "test_util/TestFileCreator.h"

using namespace ::testing;

class tst_TeArchiveFinder : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(tmpDir.isValid());
        archivePath = tmpDir.path() + "/test.zip";
        srcDir      = tmpDir.path() + "/src";

        // Build a small file tree then archive it.
        // Use absolute paths for both src and dest to ensure addEntry succeeds.
        QDir d;
        d.mkpath(srcDir);
        d.mkpath(srcDir + "/sub");
        writeFile(srcDir + "/alpha.txt", "hello");
        writeFile(srcDir + "/beta.cpp", "int main(){}");
        writeFile(srcDir + "/sub/gamma.txt", "world");

        TeArchive::Writer writer;
        // addEntry(src_absolute_path, dest_in_archive) — dest must NOT start with /
        ASSERT_TRUE(writer.addEntry(srcDir + "/alpha.txt", "alpha.txt"));
        writer.addEntry(srcDir + "/beta.cpp", "beta.cpp");
        writer.addEntry(srcDir + "/sub/gamma.txt", "sub/gamma.txt");
        ASSERT_TRUE(writer.archive(archivePath, TeArchive::AR_ZIP));
    }

    static void writeFile(const QString& path, const QByteArray& content) {
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write(content);
    }

    QTemporaryDir tmpDir;
    QString archivePath;
    QString srcDir;
};

// ── isValid ───────────────────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFinder, isValid_existing_archive_returns_true)
{
    TeArchiveFinder finder(archivePath);
    EXPECT_TRUE(finder.isValid());
}

TEST_F(tst_TeArchiveFinder, isValid_nonexistent_path_returns_false)
{
    TeArchiveFinder finder("/no_such_archive_12345.zip");
    EXPECT_FALSE(finder.isValid());
}

// ── searchFinished signal ─────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFinder, search_emits_finished_signal)
{
    TeArchiveFinder finder(archivePath);
    ASSERT_TRUE(finder.isValid());

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    // targetPath is not used to filter archive entries (they have relative paths)
    finder.startSearch(q);

    EXPECT_TRUE(spy.wait(5000));
    EXPECT_EQ(spy.count(), 1);
}

// ── results ───────────────────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFinder, no_filter_finds_all_entries)
{
    TeArchiveFinder finder(archivePath);

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    // leave targetPath empty so archive entries (relative paths) are not filtered out
    finder.startSearch(q);
    ASSERT_TRUE(spy.wait(5000));

    QList<TeFileInfo> res = finder.results();
    EXPECT_GE(res.size(), 3);
}

TEST_F(tst_TeArchiveFinder, name_filter_narrows_results)
{
    TeArchiveFinder finder(archivePath);

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    // leave targetPath empty so archive entries (relative paths) are not filtered out
    q.hasNameFilter = true;
    q.namePattern   = QRegularExpression(".*\\.txt");
    finder.startSearch(q);
    ASSERT_TRUE(spy.wait(5000));

    QList<TeFileInfo> res = finder.results();
    EXPECT_GE(res.size(), 2);
    for (const auto& fi : res) {
        EXPECT_TRUE(fi.path.endsWith(".txt"));
    }
}

// ── reset clears results ──────────────────────────────────────────────────────

TEST_F(tst_TeArchiveFinder, reset_clears_results)
{
    TeArchiveFinder finder(archivePath);

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    finder.startSearch(q);
    ASSERT_TRUE(spy.wait(5000));

    EXPECT_GT(finder.resultCount(), 0);
    finder.reset();
    EXPECT_EQ(finder.resultCount(), 0);
}
