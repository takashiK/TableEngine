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

#pragma once

/**
 * @file TeSettings.h
 * @brief Application settings constants and QSettings key strings.
 * @ingroup main
 *
 * @details Declares the TeSettings namespace with numeric limits and
 * QSettings key-path constants.  The SETTING_STR macro expands to either
 * a definition or an extern declaration depending on whether
 * _STTING_STR_DEF_ is defined before inclusion.
 */

/**
 * @namespace TeSettings
 * @brief Compile-time constants and QSettings key paths.
 */
namespace TeSettings {
	/**
	 * @brief Limits for list-style settings.
	 */
	enum {
		MAX_FAVORITES = 99, ///< Maximum number of favourite folder entries.
		MAX_HISTORY   = 99, ///< Maximum number of navigation history entries.
	};

	/**
	 * @brief Startup folder initialisation behaviour.
	 */
	enum InitFolderMode {
		INIT_FOLDER_MODE_PREVIOUS, ///< Restore last-used folder on startup.
		INIT_FOLDER_MODE_SELECTED, ///< Use the configured initial folder on startup.
	};

	/**
	 * @brief Startup window size handling mode.
	 */
	enum WindowSizeMode {
		WINDOW_SIZE_MODE_REMEMBER = 0, ///< Reuse the last window size at startup.
		WINDOW_SIZE_MODE_FIXED    = 1, ///< Always use the configured fixed size.
	};

	/** @brief Returns shared minimum width used by dialogs. */
	int dialogMinimumWidth();
};

#ifdef _STTING_STR_DEF_
#define SETTING_STR( name , str) const char* name = str
#else
#define SETTING_STR( name , str) extern const char* name
#endif

SETTING_STR(SETTING_STARTUP, "startup");

SETTING_STR(SETTING_STARTUP_MultiInstance , "startup/multi_instance" );
SETTING_STR(SETTING_STARTUP_InitialFolderMode, "startup/initial_folder_mode");
SETTING_STR(SETTING_STARTUP_InitialFolder, "startup/initial_folder");

SETTING_STR(SETTING_LAYOUT, "layout");
SETTING_STR(SETTING_LAYOUT_WINDOW_SIZE_MODE, "layout/window_size_mode");
SETTING_STR(SETTING_LAYOUT_WINDOW_FIXED_WIDTH, "layout/window_fixed_width");
SETTING_STR(SETTING_LAYOUT_WINDOW_FIXED_HEIGHT, "layout/window_fixed_height");
SETTING_STR(SETTING_LAYOUT_WINDOW_LAST_WIDTH, "layout/window_last_width");
SETTING_STR(SETTING_LAYOUT_WINDOW_LAST_HEIGHT, "layout/window_last_height");
SETTING_STR(SETTING_LAYOUT_TREE_MIN_WIDTH, "layout/tree_min_width");
SETTING_STR(SETTING_LAYOUT_TREE_MAX_WIDTH, "layout/tree_max_width");
SETTING_STR(SETTING_LAYOUT_TREE_LIST_RATIO, "layout/tree_list_ratio");
SETTING_STR(SETTING_LAYOUT_DETAIL_MIN_WIDTH, "layout/detail_min_width");
SETTING_STR(SETTING_LAYOUT_DETAIL_MAX_WIDTH, "layout/detail_max_width");
SETTING_STR(SETTING_LAYOUT_DIALOG_MIN_WIDTH, "layout/dialog_min_width");
SETTING_STR(SETTING_LAYOUT_PANE_ADJUST_WINDOW, "layout/pane_adjust_window");

SETTING_STR(SETTING_KEY, "command_key");
SETTING_STR(SETTING_TOOLBAR, "toolbar");
SETTING_STR(SETTING_MENU, "menu");
SETTING_STR(SETTING_MENUBAR_GROUP, "menuGroup00");
SETTING_STR(SETTING_TREEPOPUP_GROUP, "menuGroup01");
SETTING_STR(SETTING_LISTPOPUP_GROUP, "menuGroup02");

SETTING_STR(SETTING_FAVORITES, "Favorites");

// --- folder view appearance (extendable per section) ---
SETTING_STR(SETTING_FOLDER_APPEARANCE,       "folder/appearance");

SETTING_STR(SETTING_FOLDER_PRIO_STYLESHEET,  "folder/appearance/priority_stylesheet");

SETTING_STR(SETTING_FOLDER_FONT_FAMILY,      "folder/appearance/font_family");
SETTING_STR(SETTING_FOLDER_FONT_SIZE,        "folder/appearance/font_size");

SETTING_STR(SETTING_FOLDER_NORMAL_FG,        "folder/appearance/normal/fg_color");
SETTING_STR(SETTING_FOLDER_NORMAL_BG,        "folder/appearance/normal/bg_color");

SETTING_STR(SETTING_FOLDER_SELECTED_FG,      "folder/appearance/selected/fg_color");
SETTING_STR(SETTING_FOLDER_SELECTED_BG,      "folder/appearance/selected/bg_color");

SETTING_STR(SETTING_FOLDER_FOCUS_BG,         "folder/appearance/focus/bg_color");

// "focus" (default) → focus style wins when item is both selected and focused
// "selected"        → selected style wins when item is both selected and focused
SETTING_STR(SETTING_FOLDER_FOCUS_PRIORITY,   "folder/appearance/focus_priority");

SETTING_STR(SETTING_FOLDER_ACCENT_COLOR,     "folder/appearance/accent_color");

#undef SETTING_STR
