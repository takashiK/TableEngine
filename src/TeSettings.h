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
