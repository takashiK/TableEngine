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
#include "utils/TeHistory.h"

using namespace ::testing;

class tst_TeHistory : public ::testing::Test {
protected:
    TeHistory h;
    using PathPair = TeHistory::PathPair;
};

// ── push / current ──────────────────────────────────────────────────────────

TEST_F(tst_TeHistory, push_single_becomes_current)
{
    h.push(PathPair("/root", "/root/a"));
    EXPECT_EQ(h.current(), PathPair("/root", "/root/a"));
}

TEST_F(tst_TeHistory, push_multiple_last_is_current)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    EXPECT_EQ(h.current(), PathPair("/r", "/r/b"));
}

TEST_F(tst_TeHistory, current_on_empty_returns_empty_pair)
{
    PathPair cur = h.current();
    EXPECT_TRUE(cur.first.isEmpty());
    EXPECT_TRUE(cur.second.isEmpty());
}

// ── previous / next ──────────────────────────────────────────────────────────

TEST_F(tst_TeHistory, previous_moves_cursor_back)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    PathPair prev = h.previous();
    EXPECT_EQ(prev, PathPair("/r", "/r/a"));
    EXPECT_EQ(h.current(), PathPair("/r", "/r/a"));
}

TEST_F(tst_TeHistory, next_moves_cursor_forward)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    h.previous();
    PathPair nxt = h.next();
    EXPECT_EQ(nxt, PathPair("/r", "/r/b"));
    EXPECT_EQ(h.current(), PathPair("/r", "/r/b"));
}

TEST_F(tst_TeHistory, previous_at_beginning_returns_empty)
{
    h.push(PathPair("/r", "/r/a"));
    PathPair prev = h.previous();
    EXPECT_TRUE(prev.first.isEmpty());
    EXPECT_TRUE(prev.second.isEmpty());
}

TEST_F(tst_TeHistory, next_at_end_returns_empty)
{
    h.push(PathPair("/r", "/r/a"));
    PathPair nxt = h.next();
    EXPECT_TRUE(nxt.first.isEmpty());
    EXPECT_TRUE(nxt.second.isEmpty());
}

// ── push truncates future ────────────────────────────────────────────────────

TEST_F(tst_TeHistory, push_after_back_truncates_future)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    h.push(PathPair("/r", "/r/c"));
    h.previous();          // cursor -> b
    h.push(PathPair("/r", "/r/x")); // truncates c, appends x
    EXPECT_EQ(h.current(), PathPair("/r", "/r/x"));
    PathPair nxt = h.next(); // should be empty; c was discarded
    EXPECT_TRUE(nxt.first.isEmpty());
}

// ── pop ──────────────────────────────────────────────────────────────────────

TEST_F(tst_TeHistory, pop_returns_current_and_shrinks_list)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    PathPair popped = h.pop();
    EXPECT_EQ(popped, PathPair("/r", "/r/b"));
    EXPECT_EQ(h.get().size(), 1);
}

// ── clear ────────────────────────────────────────────────────────────────────

TEST_F(tst_TeHistory, clear_empties_all)
{
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    h.clear();
    EXPECT_TRUE(h.get().isEmpty());
    EXPECT_TRUE(h.current().first.isEmpty());
}

// ── get / getPair / set ──────────────────────────────────────────────────────

TEST_F(tst_TeHistory, get_returns_current_paths_in_order)
{
    // History is stored newest-first (prepend); get() returns in that order
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    QStringList paths = h.get();
    EXPECT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], QString("/r/b"));
    EXPECT_EQ(paths[1], QString("/r/a"));
}

TEST_F(tst_TeHistory, getPair_returns_all_pairs)
{
    // History stored newest-first
    h.push(PathPair("/r", "/r/a"));
    h.push(PathPair("/r", "/r/b"));
    auto pairs = h.getPair();
    EXPECT_EQ(pairs.size(), 2);
    EXPECT_EQ(pairs[0], PathPair("/r", "/r/b"));
    EXPECT_EQ(pairs[1], PathPair("/r", "/r/a"));
}

TEST_F(tst_TeHistory, set_replaces_list_and_resets_cursor)
{
    h.push(PathPair("/old", "/old/x"));
    QList<PathPair> newList = {
        PathPair("/n", "/n/1"),
        PathPair("/n", "/n/2"),
    };
    h.set(newList);
    EXPECT_EQ(h.getPair(), newList);
    // After set(), index is 0 (newest/first item); previous() navigates to
    // the second item (older), not empty.
    PathPair cur = h.current();
    EXPECT_EQ(cur, PathPair("/n", "/n/1"));
}

TEST_F(tst_TeHistory, set_empty_list_clears_history)
{
    h.push(PathPair("/r", "/r/a"));
    h.set({});
    EXPECT_TRUE(h.get().isEmpty());
}
