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

namespace TeSettings {
	enum {
		MAX_FAVORITES = 99,
		MAX_HISTORY   = 99,
	};

	enum InitFolderMode {
		INIT_FOLDER_MODE_PREVIOUS,
		INIT_FOLDER_MODE_SELECTED,
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
