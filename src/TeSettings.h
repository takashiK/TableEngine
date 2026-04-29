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

SETTING_STR(SETTING_KEY, "command_key");
SETTING_STR(SETTING_MENU, "menu");
SETTING_STR(SETTING_MENUBAR_GROUP, "menuGroup00");
SETTING_STR(SETTING_TREEPOPUP_GROUP, "menuGroup01");
SETTING_STR(SETTING_LISTPOPUP_GROUP, "menuGroup02");

SETTING_STR(SETTING_FAVORITES, "Favorites");

#undef SETTING_STR
