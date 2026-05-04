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
#include "utils/TeSearchQuery.h"

using namespace ::testing;

// ── isValid ──────────────────────────────────────────────────────────────────

TEST(tst_TeSearchQuery, isValid_empty_targetPath_returns_false)
{
    TeSearchQuery q;
    EXPECT_FALSE(q.isValid());
}

TEST(tst_TeSearchQuery, isValid_nonempty_targetPath_returns_true)
{
    TeSearchQuery q;
    q.targetPath = "/some/path";
    EXPECT_TRUE(q.isValid());
}

// ── default values ───────────────────────────────────────────────────────────

TEST(tst_TeSearchQuery, defaults_recursive_true)
{
    TeSearchQuery q;
    EXPECT_TRUE(q.recursive);
}

TEST(tst_TeSearchQuery, defaults_all_filters_disabled)
{
    TeSearchQuery q;
    EXPECT_FALSE(q.hasNameFilter);
    EXPECT_FALSE(q.hasSizeFilter);
    EXPECT_FALSE(q.hasDateFilter);
}

TEST(tst_TeSearchQuery, defaults_sizeLessThan_true)
{
    TeSearchQuery q;
    EXPECT_TRUE(q.sizeLessThan);
}

TEST(tst_TeSearchQuery, defaults_sizeThreshold_zero)
{
    TeSearchQuery q;
    EXPECT_EQ(q.sizeThreshold, 0);
}

// ── name filter ──────────────────────────────────────────────────────────────

TEST(tst_TeSearchQuery, namePattern_can_be_set)
{
    TeSearchQuery q;
    q.hasNameFilter = true;
    q.namePattern   = QRegularExpression(".*\\.cpp");
    EXPECT_TRUE(q.namePattern.isValid());
    EXPECT_TRUE(q.namePattern.match("foo.cpp").hasMatch());
    EXPECT_FALSE(q.namePattern.match("foo.h").hasMatch());
}

// ── size filter ───────────────────────────────────────────────────────────────

TEST(tst_TeSearchQuery, size_filter_fields)
{
    TeSearchQuery q;
    q.hasSizeFilter  = true;
    q.sizeThreshold  = 1024;
    q.sizeLessThan   = false;
    EXPECT_EQ(q.sizeThreshold, 1024);
    EXPECT_FALSE(q.sizeLessThan);
}

// ── date filter ───────────────────────────────────────────────────────────────

TEST(tst_TeSearchQuery, date_filter_fields)
{
    TeSearchQuery q;
    q.hasDateFilter = true;
    q.dateFrom = QDateTime(QDate(2024, 1, 1), QTime(0, 0));
    q.dateTo   = QDateTime(QDate(2024, 12, 31), QTime(23, 59));
    EXPECT_TRUE(q.dateFrom < q.dateTo);
    EXPECT_TRUE(q.hasDateFilter);
}

// ── combined filters still require targetPath for isValid ────────────────────

TEST(tst_TeSearchQuery, isValid_with_filters_but_no_path_returns_false)
{
    TeSearchQuery q;
    q.hasNameFilter = true;
    q.namePattern   = QRegularExpression(".*");
    EXPECT_FALSE(q.isValid());
}
