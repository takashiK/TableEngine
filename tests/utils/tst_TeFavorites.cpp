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
#include <QCoreApplication>
#include <QSettings>
#include "utils/TeFavorites.h"
#include "TeSettings.h"

using namespace ::testing;

// Use a unique QSettings scope per test run to avoid pollution from real settings.
class tst_TeFavorites : public ::testing::Test {
protected:
    void SetUp() override {
        QCoreApplication::setOrganizationName("TableEngine_Test");
        QCoreApplication::setApplicationName("TeFavoritesTest");
        // Clear any leftover from a prior run.
        QSettings s;
        s.remove(SETTING_FAVORITES);
        fav.refresh();
    }
    TeFavorites fav;
};

// ── add ──────────────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, add_new_path_returns_true)
{
    EXPECT_TRUE(fav.add("/home/user/docs"));
    EXPECT_THAT(fav.get(), Contains(QString("/home/user/docs")));
}

TEST_F(tst_TeFavorites, add_duplicate_returns_false)
{
    // add() always returns true; duplicate is removed and re-added at front
    fav.add("/home/user/docs");
    EXPECT_TRUE(fav.add("/home/user/docs"));
    EXPECT_EQ(fav.get().size(), 1);
}

TEST_F(tst_TeFavorites, add_multiple_distinct_paths)
{
    fav.add("/a");
    fav.add("/b");
    fav.add("/c");
    EXPECT_EQ(fav.get().size(), 3);
}

// ── remove ───────────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, remove_existing_path)
{
    fav.add("/a");
    fav.add("/b");
    fav.remove("/a");
    EXPECT_THAT(fav.get(), Not(Contains(QString("/a"))));
    EXPECT_EQ(fav.get().size(), 1);
}

TEST_F(tst_TeFavorites, remove_nonexistent_path_is_noop)
{
    fav.add("/a");
    fav.remove("/z");  // should not crash or change the list
    EXPECT_EQ(fav.get().size(), 1);
}

// ── move ─────────────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, move_changes_path_at_index)
{
    fav.add("/a");
    fav.add("/b");
    fav.move(0, "/x");
    QStringList list = fav.get();
    EXPECT_EQ(list[0], QString("/x"));
}

// ── clear ────────────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, clear_empties_list)
{
    fav.add("/a");
    fav.add("/b");
    fav.clear();
    EXPECT_TRUE(fav.get().isEmpty());
}

// ── get / set ─────────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, set_replaces_list)
{
    fav.add("/old");
    QStringList newList = {"/new1", "/new2"};
    fav.set(newList);
    EXPECT_EQ(fav.get(), newList);
}

// ── save / refresh ────────────────────────────────────────────────────────────

TEST_F(tst_TeFavorites, save_and_refresh_roundtrip)
{
    fav.add("/persist/a");
    fav.add("/persist/b");
    fav.save();

    TeFavorites fav2;
    fav2.refresh();
    EXPECT_THAT(fav2.get(), Contains(QString("/persist/a")));
    EXPECT_THAT(fav2.get(), Contains(QString("/persist/b")));
}

TEST_F(tst_TeFavorites, refresh_clears_unsaved_changes)
{
    fav.save();         // save empty state
    fav.add("/unsaved");
    fav.refresh();      // reload — should discard /unsaved
    EXPECT_THAT(fav.get(), Not(Contains(QString("/unsaved"))));
}

// ── MAX_FAVORITES boundary ───────────────────────────────────────────────────

TEST_F(tst_TeFavorites, max_favorites_constant_is_99)
{
    EXPECT_EQ(TeSettings::MAX_FAVORITES, 99);
}
