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

#include <QObject>

class TeTypes : public QObject
{
	Q_GADGET
public:
	enum WidgetType {
		WT_NONE,
		WT_MAINWINDOW,
		WT_MENU,
		WT_TOOLBAR,
		WT_STATUSBAR,
		WT_TAB,
		WT_FOLDERVIEW,
		WT_TREEVIEW,
		WT_LISTVIEW,
	};
	Q_ENUM(WidgetType)

	enum CmdId {
		CMDID_NONE,
		CMDID_SYSTEM         = 0x00000000,
		CMDID_SYSTEM_FILE    = CMDID_SYSTEM + 0x1000,     // Menu Item : File
		CMDID_SYSTEM_FILE_OPEN,                           // Open file
		CMDID_SYSTEM_FILE_COPY_TO,                        // #Copy to folder
		CMDID_SYSTEM_FILE_MOVE_TO,                        // #Move to folder
		CMDID_SYSTEM_FILE_DELETE,                         // #delete file or folder
		CMDID_SYSTEM_FILE_CREATE_SHORTCUT,                // #Create shoutcut
		CMDID_SYSTEM_FILE_RENAME,                         // #Rename file or folder
		CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE,               // #Change attribute
		CMDID_SYSTEM_FILE_RENAME_MULTI,                   // #Rename multi files and folders
		CMDID_SYSTEM_FILE_DIVIDE,                         // #Divde file
		CMDID_SYSTEM_FILE_COMBINE,                        // #Combine file
		CMDID_SYSTEM_FILE_ARCHIVE,                        // #Archive files
		CMDID_SYSTEM_FILE_EXTRACT,                        // #Extranct form archive
		CMDID_SYSTEM_FILE_RUN_WITH_COMMAND,               // #Execute
		CMDID_SYSTEM_FILE_EXIT,                           // #Quit TableEngine

		CMDID_SYSTEM_EDIT    = CMDID_SYSTEM + 0x2000,     // Menu Item : Edit
		CMDID_SYSTEM_EDIT_CUT,                            // Cut
		CMDID_SYSTEM_EDIT_COPY,                           // Copy
		CMDID_SYSTEM_EDIT_PASTE,                          // Paste
		CMDID_SYSTEM_EDIT_PASTE_AS_SHORTCUT,              // Paste as shotcut
		CMDID_SYSTEM_EDIT_SELECT_ALL,                     // #Change select All
		CMDID_SYSTEM_EDIT_SELECT_TOGGLE,                  // #Toggle select
		CMDID_SYSTEM_EDIT_SELECT_BY_NAME,                 // #Select by name

		CMDID_SYSTEM_FOLDER  = CMDID_SYSTEM + 0x3000,     // Menu Item : Folder
		CMDID_SYSTEM_FOLDER_OPEN_ONE,                     // #Expand current folder
		CMDID_SYSTEM_FOLDER_OPEN_UNDER,                   // #Expand current folder and its children
		CMDID_SYSTEM_FOLDER_OPEN_ALL,                     // #Expand all folders
		CMDID_SYSTEM_FOLDER_CLOSE_UNDER,                  // #Collapse current folder and its children
		CMDID_SYSTEM_FOLDER_CLOSE_ALL,                    // #Collapse all folders
		CMDID_SYSTEM_FOLDER_CREATE_FOLDER,                // #Create folder
		CMDID_SYSTEM_FOLDER_ADD_FAVORITES,                // #Add folder to favorites
		CMDID_SYSTEM_FOLDER_DEL_FAVORITES,                // Delete folder from favorites
		CMDID_SYSTEM_FOLDER_PREV_FOLDER,                  // Back to previous folder
		CMDID_SYSTEM_FOLDER_NEXT_FOLDER,                  // Go to next folder
		CMDID_SYSTEM_FOLDER_GOTO_ROOT,                    // Go to root folder
		CMDID_SYSTEM_FOLDER_GOTO_FAVORITE,                // Go to favarite folder
		CMDID_SYSTEM_FOLDER_GOTO_HISTORY,                 // Go by history
		CMDID_SYSTEM_FOLDER_GOTO_FOLDER,                  // Go to folder
		CMDID_SYSTEM_FOLDER_FIND,                         // find files

		CMDID_SYSTEM_VIEW    = CMDID_SYSTEM + 0x4000,     // Menu item : View


		CMDID_SYSTEM_TOOL    = CMDID_SYSTEM + 0x5000,     // Menu item : Tool

		CMDID_SYSTEM_WINDOW  = CMDID_SYSTEM + 0x6000,     // Menu item : Window

		CMDID_SYSTEM_SETTING = CMDID_SYSTEM + 0x7000,     // Menu item : Setting
		CMDID_SYSTEM_SETTING_OPTION,                      // #Global Setting
		CMDID_SYSTEM_SETTING_KEY,                         // #Key Setting
		CMDID_SYSTEM_SETTING_TOOLBAR,                     // Toolber setting
		CMDID_SYSTEM_SETTING_MENU,                        // #Popup & menu setting
		CMDID_SYSTEM_SETTING_STYLE,                       // Style Setting
		CMDID_SYSTEM_SETTING_USER_COMMAND,                // Regist user commands

		CMDID_SYSTEM_HELP    = CMDID_SYSTEM + 0x8000,     // Menu item : Help
		CMDID_SYSTEM_HELP_SHOW,
		CMDID_SYSTEM_HELP_VERSION,
		
		CMDID_USER        = 0xD000,                       // User registed commans

		CMDID_SPECIAL     = 0xE000,                       // Special IDs
		CMDID_SPECIAL_FOLDER,                             // Folder for menu item
		CMDID_SPECIAL_SEPARATOR,                          // Separetor for menu item

		CMDID_MASK_TYPE   = 0xF000,
	};

	Q_ENUM(CmdId)

	TeTypes();
	~TeTypes();
};

