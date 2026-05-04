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
#include "utils/TeFileFinder.h"
#include "utils/TeSearchQuery.h"

using namespace ::testing;

class tst_TeFileFinder : public ::testing::Test {
protected:
    // Creates a minimal file tree under a QTemporaryDir.
    void buildTree() {
        ASSERT_TRUE(dir.isValid());
        QString base = dir.path();
        QDir d(base);
        d.mkpath("sub");
        writeFile(base + "/alpha.txt", "hello");
        writeFile(base + "/beta.cpp", "int main(){}");
        writeFile(base + "/sub/gamma.txt", "world");
    }

    static void writeFile(const QString& path, const QByteArray& content) {
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write(content);
    }

    QTemporaryDir dir;
};

// ── isValid ───────────────────────────────────────────────────────────────────

TEST_F(tst_TeFileFinder, isValid_existing_directory_returns_true)
{
    ASSERT_TRUE(dir.isValid());
    TeFileFinder finder(dir.path());
    EXPECT_TRUE(finder.isValid());
}

TEST_F(tst_TeFileFinder, isValid_nonexistent_path_returns_false)
{
    TeFileFinder finder("/nonexistent_path_zzz_12345");
    EXPECT_FALSE(finder.isValid());
}

// ── searchFinished signal ─────────────────────────────────────────────────────

TEST_F(tst_TeFileFinder, search_emits_finished_signal)
{
    buildTree();
    TeFileFinder finder(dir.path());
    ASSERT_TRUE(finder.isValid());

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    q.targetPath = dir.path();
    finder.startSearch(q);

    EXPECT_TRUE(spy.wait(5000));
    EXPECT_EQ(spy.count(), 1);
}

// ── results contain expected files ────────────────────────────────────────────

TEST_F(tst_TeFileFinder, no_filter_finds_all_files)
{
    buildTree();
    TeFileFinder finder(dir.path());

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    q.targetPath = dir.path();
    finder.startSearch(q);
    ASSERT_TRUE(spy.wait(5000));

    QList<TeFileInfo> res = finder.results();
    EXPECT_GE(res.size(), 3); // alpha.txt, beta.cpp, sub/gamma.txt at minimum
}

TEST_F(tst_TeFileFinder, name_filter_narrows_results)
{
    buildTree();
    TeFileFinder finder(dir.path());

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    q.targetPath    = dir.path();
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

// ── cancelSearch emits searchCancelled ────────────────────────────────────────

TEST_F(tst_TeFileFinder, cancel_emits_cancelled_or_finished)
{
    buildTree();
    // Populate with many small files to give cancellation a chance.
    for (int i = 0; i < 20; ++i) {
        writeFile(dir.path() + QString("/file%1.dat").arg(i), "x");
    }

    TeFileFinder finder(dir.path());

    QSignalSpy spyCancelled(&finder, &TeFinder::searchCancelled);
    QSignalSpy spyFinished(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    q.targetPath = dir.path();
    finder.startSearch(q);
    finder.cancelSearch();

    // One of the two signals must arrive within 5 s.
    bool received = spyCancelled.wait(5000) || spyFinished.count() > 0;
    EXPECT_TRUE(received);
}

// ── reset clears results ──────────────────────────────────────────────────────

TEST_F(tst_TeFileFinder, reset_clears_results)
{
    buildTree();
    TeFileFinder finder(dir.path());

    QSignalSpy spy(&finder, &TeFinder::searchFinished);

    TeSearchQuery q;
    q.targetPath = dir.path();
    finder.startSearch(q);
    ASSERT_TRUE(spy.wait(5000));
    EXPECT_GT(finder.resultCount(), 0);

    finder.reset();
    EXPECT_EQ(finder.resultCount(), 0);
}
