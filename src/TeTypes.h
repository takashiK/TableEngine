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

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QPair>

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
		WT_ARCHIVEVIEW,
		WT_DRIVEBAR,
	};
	Q_ENUM(WidgetType)

	enum SelectionMode {
		SELECTION_NONE,
		SELECTION_EXPLORER,
		SELECTION_TABLE_ENGINE
	};

	Q_ENUM(SelectionMode)

	enum CmdId {
		CMDID_NONE,
		CMDID_SYSTEM         = 0x00000000,
		CMDID_SYSTEM_FILE    = CMDID_SYSTEM + 0x1000,     // Menu Item : File
		CMDID_SYSTEM_FILE_NEW,                            // Create file
		CMDID_SYSTEM_FILE_OPEN,                           // #Open file
		CMDID_SYSTEM_FILE_COPY_TO,                        // #Copy to folder
		CMDID_SYSTEM_FILE_MOVE_TO,                        // #Move to folder
		CMDID_SYSTEM_FILE_DELETE,                         // #delete file or folder
		CMDID_SYSTEM_FILE_RENAME,                         // #Rename file or folder
		CMDID_SYSTEM_FILE_RENAME_MULTI,                   // #Rename multi files and folders with numbering
		CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE,               // #Change attribute
		CMDID_SYSTEM_FILE_DIVIDE,                         // #Divde file
		CMDID_SYSTEM_FILE_COMBINE,                        // #Combine file
		CMDID_SYSTEM_FILE_ARCHIVE,                        // #Archive files
		CMDID_SYSTEM_FILE_EXTRACT,                        // #Extranct form archive
		CMDID_SYSTEM_FILE_RUN_WITH_COMMAND,               // #Execute Command with selected files
		CMDID_SYSTEM_FILE_PROPERTY,                       // #Property
		CMDID_SYSTEM_FILE_EXIT,                           // #Quit TableEngine

		CMDID_SYSTEM_EDIT    = CMDID_SYSTEM + 0x2000,     // Menu Item : Edit
		CMDID_SYSTEM_EDIT_CUT,                            // #Cut
		CMDID_SYSTEM_EDIT_COPY,                           // #Copy
		CMDID_SYSTEM_EDIT_PASTE,                          // #Paste
		CMDID_SYSTEM_EDIT_SELECT_ALL,                     // #Change select All
		CMDID_SYSTEM_EDIT_SELECT_TOGGLE,                  // #Toggle select
		CMDID_SYSTEM_EDIT_SELECT_BY_FILTER,               // #Select by filter

		CMDID_SYSTEM_FOLDER  = CMDID_SYSTEM + 0x3000,     // Menu Item : Folder
		CMDID_SYSTEM_FOLDER_OPEN_ONE,                     // #Expand current folder
		CMDID_SYSTEM_FOLDER_OPEN_UNDER,                   // #Expand current folder and its children
		CMDID_SYSTEM_FOLDER_OPEN_ALL,                     // #Expand all folders
		CMDID_SYSTEM_FOLDER_CLOSE_UNDER,                  // #Collapse current folder and its children
		CMDID_SYSTEM_FOLDER_CLOSE_ALL,                    // #Collapse all folders
		CMDID_SYSTEM_FOLDER_CREATE_FOLDER,                // #Create folder
		CMDID_SYSTEM_FOLDER_ADD_FAVORITE,                 // #Add folder to favorite
		CMDID_SYSTEM_FOLDER_EDIT_FAVORITES,               // #Edit favorites folder list
		CMDID_SYSTEM_FOLDER_PREV_FOLDER,                  // #Back to previous folder
		CMDID_SYSTEM_FOLDER_NEXT_FOLDER,                  // #Go to next folder
		CMDID_SYSTEM_FOLDER_GOTO_PARENT,				  // #Go to parent folder
		CMDID_SYSTEM_FOLDER_GOTO_ROOT,                    // #Go to root folder
		CMDID_SYSTEM_FOLDER_GOTO_FOLDER,                  // #Go to folder
		CMDID_SYSTEM_FOLDER_CHANGE_ROOT,                  // #Change root folder
		CMDID_SYSTEM_FOLDER_FIND,                         // #find files

		CMDID_SYSTEM_VIEW    = CMDID_SYSTEM + 0x4000,     // Menu item : View


		CMDID_SYSTEM_TOOL    = CMDID_SYSTEM + 0x5000,     // Menu item : Tool
		CMDID_SYSTEM_TOOL_VIEW_FILE,					  // #View file
		CMDID_SYSTEM_TOOL_VIEW_BINARY,					  // #View binary
		CMDID_SYSTEM_TOOL_SETUP,                          // Setup Tools
		CMDID_SYSTEM_TOOL_EXECUTE,                        // Execute Tool with parameters

		CMDID_SYSTEM_WINDOW  = CMDID_SYSTEM + 0x6000,     // Menu item : Window
		CMDID_SYSTEM_WINDOW_TOOLBAR,                      // Show ToolBar
		CMDID_SYSTEM_WINDOW_DRIVEBAR,                     // Show DriveBar
		CMDID_SYSTEM_WINDOW_STATUSBAR,                    // Show StatusBar
		CMDID_SYSTEM_WINDOW_NAVIGATION,                   // Show Navigation
		CMDID_SYSTEM_WINDOW_DETAIL,                       // Show Detail
		CMDID_SYSTEM_WINDOW_NEW_TAB,		              // New Tab
		CMDID_SYSTEM_WINDOW_CLOSE_TAB,                    // Close Tab
		CMDID_SYSTEM_WINDOW_MOVE_TAB,                     // Move Tab

		CMDID_SYSTEM_SETTING = CMDID_SYSTEM + 0x7000,     // Menu item : Setting
		CMDID_SYSTEM_SETTING_OPTION,                      // #Global Setting
		CMDID_SYSTEM_SETTING_KEY,                         // #Key Setting
		CMDID_SYSTEM_SETTING_TOOLBAR,                     // Toolber setting
		CMDID_SYSTEM_SETTING_MENU,                        // #Popup & menu setting
		CMDID_SYSTEM_SETTING_STYLE,                       // Style Setting

		CMDID_SYSTEM_HELP    = CMDID_SYSTEM + 0x8000,     // Menu item : Help
		CMDID_SYSTEM_HELP_SHOW,
		CMDID_SYSTEM_HELP_VERSION,
		
		CMDID_USER        = 0xD000,                       // User registed commands
		CMDID_USER_REGIST_COMMAND,                        // Regist user commands
		CMDID_USER_EXECUTE,                               // Execute User registed command with parameters

		CMDID_SPECIAL     = 0xE000,                       // Special IDs
		CMDID_SPECIAL_FOLDER,                             // Folder for menu item
		CMDID_SPECIAL_SEPARATOR,                          // Separetor for menu item

		CMDID_MASK_TYPE   = 0xF000,
	};

	Q_ENUM(CmdId)

	enum CmdType {
		CMD_TRIGGER_MASK       = 0xFF,
		CMD_TRIGGER_NORMAL     = 0x01,       // Normal Command
		CMD_TRIGGER_TOGGLE     = 0x02,       // Toggle Command
		CMD_TRIGGER_SELECTION  = 0x03,       // Selectional Command
		CMD_TRIGGER_PARAMETRINC= 0x04,       // Parametric Command

		CMD_CATEGORY_MASK      = 0xFF00,
		CMD_CATEGORY_TREE      = 0x0100,
		CMD_CATEGORY_LIST      = 0x0200,
		CMD_CATEGORY_NONE      = 0x0400,

		CMD_TARGET_MASK        = 0xFF0000,
		CMD_TARGET_FILE        = 0x010000,
		CMD_TARGET_ARCHIVE     = 0x020000,
	};

	Q_FLAG(CmdType)
	Q_DECLARE_FLAGS(CmdTypes, CmdType)

	TeTypes();
	~TeTypes();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TeTypes::CmdTypes)

typedef QMap<QString, QVariant> TeCmdParam;
typedef QPair<QString, TeCmdParam> TeMenuParam;
