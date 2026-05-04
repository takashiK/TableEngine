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
#include <QPixmapCache>
#include "utils/TeImageLoader.h"

using namespace ::testing;

class tst_TeImageLoader : public ::testing::Test {
protected:
    void SetUp() override {
        QPixmapCache::clear();
        ASSERT_TRUE(tmpDir.isValid());
    }

    static QString writePng(const QString& path) {
        // Write a minimal 1×1 PNG so Qt can decode it.
        QImage img(1, 1, QImage::Format_RGB32);
        img.fill(Qt::red);
        img.save(path, "PNG");
        return path;
    }

    QTemporaryDir tmpDir;
};

// ── cacheKey uniqueness ───────────────────────────────────────────────────────

TEST_F(tst_TeImageLoader, cache_key_differs_for_different_paths)
{
    QDateTime dt = QDateTime::currentDateTime();
    QSize sz(64, 64);
    QString k1 = TeImageLoader::cacheKey("/a/b.png", sz, dt);
    QString k2 = TeImageLoader::cacheKey("/a/c.png", sz, dt);
    EXPECT_NE(k1, k2);
}

TEST_F(tst_TeImageLoader, cache_key_differs_for_different_sizes)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString path = "/img.png";
    QString k1 = TeImageLoader::cacheKey(path, QSize(32, 32), dt);
    QString k2 = TeImageLoader::cacheKey(path, QSize(64, 64), dt);
    EXPECT_NE(k1, k2);
}

TEST_F(tst_TeImageLoader, cache_key_differs_for_different_mtimes)
{
    QSize sz(64, 64);
    QString path = "/img.png";
    QDateTime dt1 = QDateTime(QDate(2024, 1, 1), QTime(0, 0));
    QDateTime dt2 = QDateTime(QDate(2024, 1, 2), QTime(0, 0));
    EXPECT_NE(TeImageLoader::cacheKey(path, sz, dt1),
              TeImageLoader::cacheKey(path, sz, dt2));
}

TEST_F(tst_TeImageLoader, cache_key_same_for_same_inputs)
{
    QDateTime dt = QDateTime(QDate(2024, 6, 1), QTime(12, 0));
    QSize sz(64, 64);
    QString path = "/some/image.png";
    EXPECT_EQ(TeImageLoader::cacheKey(path, sz, dt),
              TeImageLoader::cacheKey(path, sz, dt));
}

// ── imageReady signal ─────────────────────────────────────────────────────────

TEST_F(tst_TeImageLoader, request_load_emits_image_ready)
{
    QString imgPath = tmpDir.path() + "/test.png";
    writePng(imgPath);

    TeImageLoader loader;
    QSignalSpy spy(&loader, &TeImageLoader::imageReady);

    loader.requestLoad(imgPath, QSize(64, 64));
    EXPECT_TRUE(spy.wait(5000));

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy[0][0].toString(), imgPath);
}

TEST_F(tst_TeImageLoader, duplicate_request_does_not_double_emit)
{
    QString imgPath = tmpDir.path() + "/test2.png";
    writePng(imgPath);

    TeImageLoader loader;
    QSignalSpy spy(&loader, &TeImageLoader::imageReady);

    // Request the same image twice back-to-back; only one load should happen.
    loader.requestLoad(imgPath, QSize(32, 32));
    loader.requestLoad(imgPath, QSize(32, 32));

    // Wait enough for both potential emissions.
    QTest::qWait(2000);
    EXPECT_LE(spy.count(), 1);
}

// ── clearPendingRequests ──────────────────────────────────────────────────────

TEST_F(tst_TeImageLoader, clear_pending_requests_does_not_crash)
{
    TeImageLoader loader;
    // Queue a few requests and immediately cancel — must not crash.
    for (int i = 0; i < 5; ++i) {
        loader.requestLoad(tmpDir.path() + QString("/x%1.png").arg(i), QSize(16, 16));
    }
    loader.clearPendingRequests();
    QTest::qWait(500);  // Let thread pool settle.
    SUCCEED();
}
