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

#include "TeCommandFactory.h"
#include "TeCommandInfo.h"

#include "file/TeCmdOpenFile.h"
#include "file/TeCmdCopyTo.h"
#include "file/TeCmdMoveTo.h"
#include "file/TeCmdDelete.h"
#include "file/TeCmdRename.h"
#include "file/TeCmdRenameMulti.h"
#include "file/TeCmdChangeAttribute.h"
#include "file/TeCmdDivideFile.h"
#include "file/TeCmdCombineFile.h"
#include "file/TeCmdArchive.h"
#include "file/TeCmdExtract.h"
#include "file/TeCmdProperties.h"
#include "file/TeCmdRunCommand.h"
#include "file/TeCmdExit.h"

#include "edit/TeCmdCut.h"
#include "edit/TeCmdCopy.h"
#include "edit/TeCmdPaste.h"
#include "edit/TeCmdSelectAll.h"
#include "edit/TeCmdSelectToggle.h"

#include "folder/TeCmdFolderCloseAll.h"
#include "folder/TeCmdFolderCloseUnder.h"
#include "folder/TeCmdFolderCreate.h"
#include "folder/TeCmdFolderOpenAll.h"
#include "folder/TeCmdFolderOpenOne.h"
#include "folder/TeCmdFolderOpenUnder.h"

#include "folder/TeCmdFolderChangeRoot.h"

#include "tool/TeCmdToolFile.h"
#include "tool/TeCmdToolBinary.h"

#include "setting/TeCmdOption.h"
#include "setting/TeCmdKeySetting.h"
#include "setting/TeCmdMenuSetting.h"

TeCommandFactory::TeCommandFactory()
{
#define BEGIN_GROUP( groupId ) { TeTypes::CmdId cur_group = groupId; int cur_rank = 1
#define END_GROUP() }while(0)

#define BEGIN_FOLDER(name) m_menuSpecs[cur_group].append({TeTypes::CMDID_SPECIAL_FOLDER,name,cur_rank}); { cur_rank += 1
#define END_FOLDER() cur_rank -= 1; }while(0)

#define SEPARATOR() m_menuSpecs[cur_group].append({TeTypes::CMDID_SPECIAL_SEPARATOR,"",cur_rank})
#define MENU_ENTRY(cmdId,cmdClass,name,description,icon)   m_menuSpecs[cur_group].append({cmdId,name,cur_rank}); m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon); m_group[cur_group].append(cmdId)
#define CMD_ENTRY(cmdId,cmdClass,name,description,icon)  m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon); m_group[cur_group].append(cmdId)



	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_FILE);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_OPEN, TeCmdOpenFile, tr("&Open"), tr("Open file by shell."), QIcon(":/TableEngine/openFile.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_COPY_TO, TeCmdCopyTo, tr("&Copy To"), tr("Copy selected files to other folder."), QIcon(":/TableEngine/copyTo.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_MOVE_TO, TeCmdMoveTo, tr("&Move To"), tr("Move selected files to other folder."), QIcon(":/TableEngine/moveTo.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_DELETE, TeCmdDelete, tr("&Delete"), tr("Delete selected files."), QIcon(":TableEngine/trash.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RENAME, TeCmdRename, tr("&Rename"), tr("Rename selected files."), QIcon(":TableEngine/rename.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RENAME_MULTI, TeCmdRenameMulti, tr("RenameMulti"), tr("Rename multiple files with filter."), QIcon(":TableEngine/renameMulti.png"));
		//MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE, TeCmdChangeAttribute, tr("Attribute"), tr("Change file attribute."), QIcon(":TableEngine/attribute.png"));
		BEGIN_FOLDER("Divide");
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_DIVIDE, TeCmdDivideFile, tr("Divide file"), tr("Divide file."), QIcon(":TableEngine/divide.png"));
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_COMBINE, TeCmdCombineFile, tr("Combine files"), tr("Combine files."), QIcon(":TableEngine/combine.png"));
		END_FOLDER();
		BEGIN_FOLDER("Archive");
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_ARCHIVE, TeCmdArchive, tr("&Archive files"), tr("Archive selected files."), QIcon(":/TableEngine/archiveBox.png"));
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_EXTRACT, TeCmdExtract, tr("&Extract files"), tr("Extract from selected file."), QIcon(":/TableEngine/extract.png"));
		END_FOLDER();
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_PROPERTY, TeCmdProperties, tr("&Properties"), tr("Show item properties."), QIcon(":/TableEngine/properties.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RUN_WITH_COMMAND, TeCmdRunCommand, tr("&Run command"), tr("Run command line."), QIcon(":/TableEngine/command.png"));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_EXIT, TeCmdExit, tr("&Quit"), tr("Quit application."), QIcon(":/TableEngine/exit.png"));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_EDIT);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_CUT, TeCmdCut, tr("Cut"), tr("Select all files."), QIcon(":/TableEngine/cut.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_COPY, TeCmdCopy, tr("Copy"), tr("Select all files."), QIcon(":/TableEngine/copy.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_PASTE, TeCmdPaste, tr("Paste"), tr("Select all files."), QIcon(":/TableEngine/paste.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_ALL, TeCmdSelectAll, tr("Select &All"), tr("Select all files."), QIcon(":/TableEngine/selectAll.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_TOGGLE, TeCmdSelectToggle, tr("&Toggle"), tr("Toggle selection."), QIcon(":/TableEngine/selectToggle.png"));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_FOLDER);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ONE, TeCmdFolderOpenOne, tr("Expand&One"), tr("Expand current Folder."), QIcon(":/TableEngine/expand.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_UNDER, TeCmdFolderOpenUnder, tr("Expand &Under"), tr("Expand current Folder and children."), QIcon(":/TableEngine/expandFollow.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ALL, TeCmdFolderOpenAll, tr("&Expand All"), tr("Expand all Folders."), QIcon(":/TableEngine/expandAll.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_UNDER, TeCmdFolderCloseUnder, tr("Colla&pse"), tr("Collapse current Folder."), QIcon(":/TableEngine/collapse.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_ALL, TeCmdFolderCloseAll, tr("&Collapse All"), tr("Collapse all Folders."), QIcon(":/TableEngine/collapseAll.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CREATE_FOLDER, TeCmdFolderCreate, tr("&New Folder"), tr("Create new folder."), QIcon(":/TableEngine/newFolder.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_ADD_FAVORITES, TeCmdFolderAddFavorites, tr("Add Favorites"), tr("Add a folder to favorites list."), QIcon(":/TableEngine/addFavorites.png");
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_DEL_FAVORITES, TeCmdFolderDelFavorites, tr("Del Favorites"), tr("Delete a folder from favorites list."), QIcon(":/TableEngine/delFavorites.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_PREV_FOLDER, TeCmdFolderPrevFolder, tr("Previous Folder"),tr("Go to previous folder."), QIcon(":/TableEngine/prevFolder.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_NEXT_FOLDER, TeCmdFolderNextFolder, tr("Next Folder"), tr("Go to next folder."), QIcon(":/TableEngine/nextFolder.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_GOTO_ROOT, TeCmdFolderGotoRoot, tr("Go to Root"), tr("Go to root folder."), QIcon(":/TableEngine/gotoRoot.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_GOTO_FOLDER, TeCmdFolderGotoFolder, tr("&Go to folder"), tr("Go to folder."), QIcon(":/TableEngine/goto.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CHANGE_ROOT, TeCmdFolderChangeRoot, tr("Change Root folder"), tr("Change root folder."), QIcon(":/TableEngine/changeRoot.png"));
		//	MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_FIND, TeCmdFolderFind, tr("Find"), tr("Find items."), QIcon(":/TableEngine/find.png"));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_VIEW);
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_TOOL);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_TOOL_VIEW_FILE, TeCmdToolFile, tr("View File"), tr("View file."), QIcon(":/TableEngine/view.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_TOOL_VIEW_BINARY, TeCmdToolBinary, tr("View Binary"), tr("View binary file."), QIcon(":/TableEngine/binary.png"));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_WINDOW);
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_SETTING);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_OPTION, TeCmdOption, tr("&Option"), tr("General options."), QIcon(":/TableEngine/settings.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_KEY, TeCmdKeySetting, tr("&Key"), tr("Customize key oparations."), QIcon(":/TableEngine/keyboard.png"));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_MENU, TeCmdMenuSetting, tr("&Menu"), tr("Customize Menu item."), QIcon(":/TableEngine/menu.png"));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_HELP);
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_USER);
	END_GROUP();

#undef BEGIN_GROUP
#undef END_GROUP
#undef BEGIN_FOLDER
#undef END_FOLDER
#undef SEPARATOR
#undef MENU_ENTRY
#undef CMD_ENTRY
}


TeCommandFactory::~TeCommandFactory()
{
}

TeCommandFactory * TeCommandFactory::factory()
{
	static TeCommandFactory factory;
	return &factory;
}

QList<QPair<QString, TeTypes::CmdId>> TeCommandFactory::groupList()
{
	QList<QPair<QString, TeTypes::CmdId>> list = {
		{ tr("&File"), TeTypes::CMDID_SYSTEM_FILE },
		{ tr("&Edit"), TeTypes::CMDID_SYSTEM_EDIT },
		{ tr("&Folder"), TeTypes::CMDID_SYSTEM_FOLDER },
		{ tr("&View"), TeTypes::CMDID_SYSTEM_VIEW },
		{ tr("&Tool"), TeTypes::CMDID_SYSTEM_TOOL },
		{ tr("&Window"), TeTypes::CMDID_SYSTEM_WINDOW },
		{ tr("&Setting"), TeTypes::CMDID_SYSTEM_SETTING },
		{ tr("&Help"), TeTypes::CMDID_SYSTEM_HELP },
		{ tr("&User"), TeTypes::CMDID_USER },
	};
	return list;
}

QList<QPair<QString, TeTypes::CmdId>> TeCommandFactory::custom_groupList()
{
	QList<QPair<QString, TeTypes::CmdId>> list = {
		{ tr("&File"), TeTypes::CMDID_SYSTEM_FILE },
		{ tr("&Edit"), TeTypes::CMDID_SYSTEM_EDIT },
		{ tr("&Folder"), TeTypes::CMDID_SYSTEM_FOLDER },
		{ tr("&View"), TeTypes::CMDID_SYSTEM_VIEW },
		{ tr("&Tool"), TeTypes::CMDID_SYSTEM_TOOL },
		{ tr("&Window"), TeTypes::CMDID_SYSTEM_WINDOW },
		{ tr("&User"), TeTypes::CMDID_USER },
	};
	return list;
}

QList<QPair<QString, TeTypes::CmdId>> TeCommandFactory::static_groupList()
{
	QList<QPair<QString, TeTypes::CmdId>> list = {
		{ tr("&Setting"), TeTypes::CMDID_SYSTEM_SETTING },
		{ tr("&Help"), TeTypes::CMDID_SYSTEM_HELP },
	};
	return list;
}

QList<const TeCommandInfoBase*> TeCommandFactory::commandGroup(TeTypes::CmdId groupId) const
{
	QList<const TeCommandInfoBase*> group;

	if (m_group.contains(groupId)) {
		for (const auto& cmdId : m_group[groupId]) {
			const TeCommandInfoBase* p_info = commandInfo(cmdId);
			if (p_info) {
				group.append(p_info);
			}
		}
	}

	return group;
}

QList<TeMenuSpec> TeCommandFactory::menuGroup(TeTypes::CmdId groupId) const
{
	if(m_menuSpecs.contains(groupId))
		return m_menuSpecs[groupId];
	return QList<TeMenuSpec>();
}



const TeCommandInfoBase* TeCommandFactory::commandInfo(TeTypes::CmdId cmdId) const
{
	TeCommandInfoBase* p_info = nullptr;
	if (m_commands.contains(cmdId)) {
		p_info = m_commands[cmdId];
	}
	return p_info;
}

TeCommandBase * TeCommandFactory::createCommand(TeTypes::CmdId cmdId) const
{
	TeCommandBase* p_cmd = nullptr;
	if (m_commands.contains(cmdId)) {
		p_cmd = m_commands[cmdId]->createCommand();
	}
	return p_cmd;
}
