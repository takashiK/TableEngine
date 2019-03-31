/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

namespace TeSettings {
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

SETTING_STR(SETTING_STARTUP_MultiInstance , "startup/multi_instance" );
SETTING_STR(SETTING_STARTUP_InitialFolderMode, "startup/initial_folder_mode");
SETTING_STR(SETTING_STARTUP_InitialFolder, "startup/initial_folder");

SETTING_STR(SETTING_KEY, "command_key");
SETTING_STR(SETTING_MENU, "menu");

#undef SETTING_STR
