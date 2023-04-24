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

#include "file/TeCmdCopyTo.h"
#include "file/TeCmdMoveTo.h"
#include "file/TeCmdDelete.h"

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

#include "view/TeCmdViewFile.h"

#include "setting/TeCmdOption.h"
#include "setting/TeCmdKeySetting.h"
#include "setting/TeCmdMenuSetting.h"

TeCommandFactory::TeCommandFactory()
{
#define CMD_ENTRY(cmdId,cmdClass,name,description,icon)  m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon)

	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FILE_COPY_TO, TeCmdCopyTo, tr("&Copy To"), tr("Copy selected files to other folder."), QIcon(":/TableEngine/copyTo.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FILE_MOVE_TO, TeCmdMoveTo, tr("&Move To"), tr("Move selected files to other folder."), QIcon(":/TableEngine/moveTo.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FILE_DELETE, TeCmdDelete, tr("&Delete"), tr("Delete selected files."), QIcon(":TableEngine/trash.png"));

	CMD_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_CUT, TeCmdCut, tr("Cut"), tr("Select all files."), QIcon(":/TableEngine/cut.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_COPY, TeCmdCopy, tr("Copy"), tr("Select all files."), QIcon(":/TableEngine/copy.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_PASTE, TeCmdPaste, tr("Paste"), tr("Select all files."), QIcon(":/TableEngine/paste.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_ALL, TeCmdSelectAll, tr("Select &All"), tr("Select all files."), QIcon(":/TableEngine/selectAll.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_TOGGLE, TeCmdSelectToggle, tr("&Toggle"), tr("Toggle selection."), QIcon(":/TableEngine/selectToggle.png"));

	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ONE, TeCmdFolderOpenOne, tr("Expand&One"), tr("Expand current Folder."), QIcon(":/TableEngine/expand.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_UNDER, TeCmdFolderOpenUnder, tr("Expand &Under"), tr("Expand current Folder and children."), QIcon(":/TableEngine/expandFollow.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ALL, TeCmdFolderOpenAll, tr("&Expand All"), tr("Expand all Folders."), QIcon(":/TableEngine/expandAll.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_UNDER, TeCmdFolderCloseUnder, tr("Colla&pse"), tr("Collapse current Folder."), QIcon(":/TableEngine/collapse.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_ALL, TeCmdFolderCloseAll, tr("&Collapse All"), tr("Collapse all Folders."), QIcon(":/TableEngine/collapseAll.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CREATE_FOLDER, TeCmdFolderCreate, tr("&New Folder"), tr("Create new folder."), QIcon(":/TableEngine/newFolder.png"));
	
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_VIEW_VIEW_FILE, TeCmdViewFile, tr("View File"), tr("View file."), QIcon(":/TableEngine/view.png"));

	CMD_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_OPTION, TeCmdOption, tr("&Option"), tr("General options."), QIcon(":/TableEngine/settings.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_KEY, TeCmdKeySetting, tr("&Key"), tr("Customize key oparations."), QIcon(":/TableEngine/keyboard.png"));
	CMD_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_MENU, TeCmdMenuSetting, tr("&Menu"), tr("Customize Menu item."), QIcon(":/TableEngine/menu.png"));

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

QList<TeCommandInfoBase*> TeCommandFactory::commandGroup(TeTypes::CmdId groupId)
{
	QList<TeCommandInfoBase*> group;

	for (auto itr = m_commands.begin(); itr != m_commands.end(); ++itr) {
		if ( (itr.key() & TeTypes::CMDID_MASK_TYPE) == groupId) {
			group.append(itr.value());
		}
	}

	return group;
}

TeCommandInfoBase* TeCommandFactory::commandInfo(TeTypes::CmdId cmdId)
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
