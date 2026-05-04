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
#include <QString>
#include "TeSettings.h"

using namespace ::testing;

// ── TeSettings enum constants ─────────────────────────────────────────────────

TEST(tst_TeSettings, max_favorites_is_99)
{
    EXPECT_EQ(TeSettings::MAX_FAVORITES, 99);
}

TEST(tst_TeSettings, max_history_is_99)
{
    EXPECT_EQ(TeSettings::MAX_HISTORY, 99);
}

TEST(tst_TeSettings, init_folder_mode_values_distinct)
{
    EXPECT_NE(TeSettings::INIT_FOLDER_MODE_PREVIOUS,
              TeSettings::INIT_FOLDER_MODE_SELECTED);
}

// ── SETTING_STR keys non-empty ────────────────────────────────────────────────

TEST(tst_TeSettings, setting_startup_key_nonempty)
{
    EXPECT_STRNE(SETTING_STARTUP, "");
}

TEST(tst_TeSettings, setting_favorites_key_nonempty)
{
    EXPECT_STRNE(SETTING_FAVORITES, "");
}

TEST(tst_TeSettings, setting_key_nonempty)
{
    EXPECT_STRNE(SETTING_KEY, "");
}

TEST(tst_TeSettings, setting_menu_key_nonempty)
{
    EXPECT_STRNE(SETTING_MENU, "");
}

TEST(tst_TeSettings, startup_multi_instance_key_contains_startup_prefix)
{
    QString key = QString::fromLatin1(SETTING_STARTUP_MultiInstance);
    EXPECT_TRUE(key.startsWith("startup"));
}
