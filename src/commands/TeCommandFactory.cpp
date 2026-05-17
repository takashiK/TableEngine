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
#include "utils/TeAdaptiveIconEngine.h"

#include "file/TeCmdFileCreate.h"
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
#include "edit/TeCmdSelectionStyle.h"
#include "edit/TeCmdSelectFilter.h"

#include "folder/TeCmdFolderCloseAll.h"
#include "folder/TeCmdFolderCloseUnder.h"
#include "folder/TeCmdFolderCreate.h"
#include "folder/TeCmdFolderOpenAll.h"
#include "folder/TeCmdFolderOpenOne.h"
#include "folder/TeCmdFolderOpenUnder.h"
#include "folder/TeCmdAddFavorite.h"
#include "folder/TeCmdEditFavorites.h"
#include "folder/TeCmdPrevFolder.h"
#include "folder/TeCmdNextFolder.h"
#include "folder/TeCmdGotoParent.h"
#include "folder/TeCmdGotoRoot.h"
#include "folder/TeCmdGotoFolder.h"
#include "folder/TeCmdFolderChangeRoot.h"
#include "folder/TeCmdFind.h"

#include "view/TeCmdViewShowInfo.h"
#include "view/TeCmdViewShowFile.h"
#include "view/TeCmdViewFilter.h"
#include "view/TeCmdViewFileOrderBy.h"
#include "view/TeCmdViewFileOrder.h"
#include "view/TeCmdViewLayout.h"

#include "tool/TeCmdToolFile.h"
#include "tool/TeCmdToolBinary.h"

#include "window/TeCmdWindowBar.h"
#include "window/TeCmdWindowNewTab.h"
#include "window/TeCmdWindowCloseTab.h"
#include "window/TeCmdWindowMoveTab.h"

#include "setting/TeCmdOption.h"
#include "setting/TeCmdKeySetting.h"
#include "setting/TeCmdMenuSetting.h"

#include "help/TeCmdHelp.h"
#include "help/TeCmdVersion.h"

/**
 * @file TeCommandFactory.cpp
 * @brief Implementation of TeCommandFactory.
 * @ingroup commands
 */

TeCommandFactory::TeCommandFactory()
{
#define BEGIN_GROUP( groupId ) { TeTypes::CmdId cur_group = groupId; int cur_rank = 1
#define END_GROUP() }while(0)

#define BEGIN_FOLDER(name) m_menuSpecs[cur_group].append({TeTypes::CMDID_SPECIAL_FOLDER,name,cur_rank}); { cur_rank += 1
#define END_FOLDER() cur_rank -= 1; }while(0)

#define SEPARATOR() m_menuSpecs[cur_group].append({TeTypes::CMDID_SPECIAL_SEPARATOR,"",cur_rank})
#define MENU_ENTRY(cmdId,cmdClass,name,description,icon)   m_menuSpecs[cur_group].append({cmdId,name,cur_rank}); m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon); m_group[cur_group].append(cmdId)
#define MENU_ENTRY_WITH_SELECT(cmdId,cmdClass, param, name,description,icon)   m_menuSpecs[cur_group].append({cmdId,name,cur_rank}); m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon, {{"SELECTION",param}}); m_group[cur_group].append(cmdId)
#define CMD_ENTRY(cmdId,cmdClass,name,description,icon)  m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon); m_group[cur_group].append(cmdId)
#define CMD_ENTRY_WITH_SELECT(cmdId,cmdClass, param, name,description,icon)  m_commands[cmdId] = new TeCommandInfo<cmdClass>(cmdId, name, description, icon, {{"SELECTION",param}}); m_group[cur_group].append(cmdId)


	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_FILE);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_NEW, TeCmdFileCreate, tr("&New"), tr("Create New file."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/newFile.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_OPEN, TeCmdOpenFile, tr("&Open"), tr("Open file by shell."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/openFile.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_COPY_TO, TeCmdCopyTo, tr("&Copy To"), tr("Copy selected files to other folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/copyTo.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_MOVE_TO, TeCmdMoveTo, tr("&Move To"), tr("Move selected files to other folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/moveTo.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_DELETE, TeCmdDelete, tr("&Delete"), tr("Delete selected files."), QIcon(new TeAdaptiveIconEngine(":TableEngine/trash.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RENAME, TeCmdRename, tr("&Rename"), tr("Rename selected files."), QIcon(new TeAdaptiveIconEngine(":TableEngine/rename.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RENAME_MULTI, TeCmdRenameMulti, tr("RenameMulti"), tr("Rename multiple files with filter."), QIcon(new TeAdaptiveIconEngine(":TableEngine/renameMulti.png")));
		//MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_CHANGE_ATTRIBUTE, TeCmdChangeAttribute, tr("Attribute"), tr("Change file attribute."), QIcon(new TeAdaptiveIconEngine(":TableEngine/attribute.png")));
		BEGIN_FOLDER("Divide");
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_DIVIDE, TeCmdDivideFile, tr("Divide file"), tr("Divide file."), QIcon(new TeAdaptiveIconEngine(":TableEngine/divide.png")));
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_COMBINE, TeCmdCombineFile, tr("Combine files"), tr("Combine files."), QIcon(new TeAdaptiveIconEngine(":TableEngine/combine.png")));
		END_FOLDER();
		BEGIN_FOLDER("Archive");
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_ARCHIVE, TeCmdArchive, tr("&Archive files"), tr("Archive selected files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/archiveBox.png")));
			MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_EXTRACT, TeCmdExtract, tr("&Extract files"), tr("Extract from selected file."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/extract.png")));
		END_FOLDER();
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_PROPERTY, TeCmdProperties, tr("&Properties"), tr("Show item properties."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/properties.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_RUN_WITH_COMMAND, TeCmdRunCommand, tr("&Run command"), tr("Run command line."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/command.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FILE_EXIT, TeCmdExit, tr("&Quit"), tr("Quit application."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/exit.png")));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_EDIT);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_CUT, TeCmdCut, tr("Cut"), tr("Select all files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/cut.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_COPY, TeCmdCopy, tr("Copy"), tr("Select all files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/copy.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_PASTE, TeCmdPaste, tr("Paste"), tr("Select all files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/paste.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_ALL, TeCmdSelectAll, tr("Select &All"), tr("Select all files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/selectAll.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_TOGGLE, TeCmdSelectToggle, tr("&Toggle"), tr("Toggle selection."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/selectToggle.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_EDIT_SELECT_BY_FILTER, TeCmdSelectFilter, tr("Select by &Filter"), tr("Select by filter."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/selectFilter.png")));
		BEGIN_FOLDER("Selection Style");
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_EDIT_SELECTION_STYLE_EXPLORER, TeCmdSelectionStyle, TeCmdSelectionStyle::Explorer, tr("&Explorer"), tr("Use explorer style selection."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/selectionExplorer.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_EDIT_SELECTION_STYLE_TABLEENGINE, TeCmdSelectionStyle, TeCmdSelectionStyle::TableEngine, tr("&TableEngine"), tr("Use TableEngine style selection."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/selectionTableEngine.png")));
		END_FOLDER();
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_FOLDER);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CREATE_FOLDER, TeCmdFolderCreate, tr("&New Folder"), tr("Create new folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/newFolder.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ONE, TeCmdFolderOpenOne, tr("Expand&One"), tr("Expand current Folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/expand.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_UNDER, TeCmdFolderOpenUnder, tr("Expand &Under"), tr("Expand current Folder and children."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/expandFollow.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_OPEN_ALL, TeCmdFolderOpenAll, tr("&Expand All"), tr("Expand all Folders."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/expandAll.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_UNDER, TeCmdFolderCloseUnder, tr("Colla&pse"), tr("Collapse current Folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/collapse.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CLOSE_ALL, TeCmdFolderCloseAll, tr("&Collapse All"), tr("Collapse all Folders."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/collapseAll.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_ADD_FAVORITE, TeCmdAddFavorite, tr("Add Favorite"), tr("Add a folder to favorites list."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/addFavorite.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_EDIT_FAVORITES, TeCmdEditFavorites, tr("Edit Favorites"), tr("Edit favorites list."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/editFavorites.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_PREV_FOLDER, TeCmdPrevFolder, tr("Previous Folder"),tr("Go to previous folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/previous.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_NEXT_FOLDER, TeCmdNextFolder, tr("Next Folder"), tr("Go to next folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/next.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_GOTO_PARENT, TeCmdGotoParent, tr("Go to Parent"), tr("Go to parent folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/parent.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_GOTO_ROOT, TeCmdGotoRoot, tr("Go to Root"), tr("Go to root folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/root.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_GOTO_FOLDER, TeCmdGotoFolder, tr("&Go to folder"), tr("Go to folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/goto.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_CHANGE_ROOT, TeCmdFolderChangeRoot, tr("Change Root folder"), tr("Change root folder."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/changeRoot.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_FOLDER_FIND, TeCmdFind, tr("Find"), tr("Find items."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/find.png")));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_VIEW);
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_FILESIZE, TeCmdViewShowInfo, TeCmdViewShowInfo::FileSize, tr("File Size"), tr("Show or hide file size."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/fileSize.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_MODIFIED, TeCmdViewShowInfo, TeCmdViewShowInfo::ModifiedDate, tr("Modified Date"), tr("Show or hide modified date."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/modifiedDate.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_SHOW_HIDDEN, TeCmdViewShowFile, TeCmdViewShowFile::Hidden, tr("Hidden Files"), tr("Show or hide hidden files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/hidden.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_SHOW_SYSTEM, TeCmdViewShowFile, TeCmdViewShowFile::System, tr("System Files"), tr("Show or hide system files."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/system.png")));
		SEPARATOR();
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_VIEW_FILTER, TeCmdViewFilter, tr("Filter"), tr("Filter files by name."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/filter.png")));
		SEPARATOR();
		BEGIN_FOLDER(tr("Sort by"));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDERBY_NAME, TeCmdViewFileOrderBy, TeCmdViewFileOrderBy::FileName, tr("Name"), tr("Sort by name."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortName.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDERBY_SIZE, TeCmdViewFileOrderBy, TeCmdViewFileOrderBy::FileSize, tr("Size"), tr("Sort by file size."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortSize.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDERBY_EXTENSION, TeCmdViewFileOrderBy, TeCmdViewFileOrderBy::Extension, tr("Extension"), tr("Sort by file extension."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortExtension.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDERBY_MODIFIED, TeCmdViewFileOrderBy, TeCmdViewFileOrderBy::ModifiedDate, tr("Modified Date"), tr("Sort by modified date."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortModifiedDate.png")));
		END_FOLDER();
		BEGIN_FOLDER(tr("Order"));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDER_ASCENDING, TeCmdViewFileOrder, TeCmdViewFileOrder::Ascending, tr("Ascending"), tr("Sort in ascending order."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortAsc.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_ORDER_DESCENDING, TeCmdViewFileOrder, TeCmdViewFileOrder::Descending, tr("Descending"), tr("Sort in descending order."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/sortDesc.png")));
		END_FOLDER();
		SEPARATOR();
		BEGIN_FOLDER(tr("Layout"));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_SMALL_ICON, TeCmdViewLayout, TeCmdViewLayout::SMALL_ICON, tr("Small Icon"), tr("Show small icons."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/smallIcon.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_LARGE_ICON, TeCmdViewLayout, TeCmdViewLayout::LARGE_ICON, tr("Large Icon"), tr("Show large icons."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/largeIcon.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_HUGE_ICON, TeCmdViewLayout, TeCmdViewLayout::HUGE_ICON, tr("Huge Icon"), tr("Show huge icons."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/hugeIcon.png")));
			MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_VIEW_DETAIL_LIST, TeCmdViewLayout, TeCmdViewLayout::DETAIL_LIST, tr("Detail List"), tr("Show detail list."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/detailList.png")));
		END_FOLDER();
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_TOOL);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_TOOL_VIEW_FILE, TeCmdToolFile, tr("View File"), tr("View file."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/view.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_TOOL_VIEW_BINARY, TeCmdToolBinary, tr("View Binary"), tr("View binary file."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/binary.png")));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_WINDOW);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_WINDOW_NEW_TAB, TeCmdWindowNewTab, tr("New Tab"), tr("Open new tab."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/newTab.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_WINDOW_CLOSE_TAB, TeCmdWindowCloseTab, tr("Close Tab"), tr("Close current tab."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/closeTab.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_WINDOW_MOVE_TAB, TeCmdWindowMoveTab, tr("Move Tab"), tr("Move current tab to other window."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/moveTab.png")));
		SEPARATOR();
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_WINDOW_TOOLBAR, TeCmdWindowBar, TeCmdWindowBar::ToolBar, tr("Show ToolBar"), tr("Show or hide ToolBar."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/toolBar.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_WINDOW_DRIVEBAR, TeCmdWindowBar, TeCmdWindowBar::DriveBar, tr("Show DriveBar"), tr("Show or hide DriveBar."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/driveBar.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_WINDOW_STATUSBAR, TeCmdWindowBar, TeCmdWindowBar::StatusBar, tr("Show StatusBar"), tr("Show or hide StatusBar."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/statusBar.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_WINDOW_NAVIGATION, TeCmdWindowBar, TeCmdWindowBar::Navigation, tr("Show Navigation"), tr("Show or hide Navigation."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/navigation.png")));
		MENU_ENTRY_WITH_SELECT(TeTypes::CMDID_SYSTEM_WINDOW_DETAIL, TeCmdWindowBar, TeCmdWindowBar::Detail, tr("Show Detail"), tr("Show or hide Detail."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/detail.png")));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_SETTING);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_OPTION, TeCmdOption, tr("&Option"), tr("General options."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/settings.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_KEY, TeCmdKeySetting, tr("&Key"), tr("Customize key oparations."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/keyboard.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_SETTING_MENU, TeCmdMenuSetting, tr("&Menu"), tr("Customize Menu item."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/menu.png")));
	END_GROUP();

	BEGIN_GROUP(TeTypes::CMDID_SYSTEM_HELP);
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_HELP_SHOW, TeCmdHelp, tr("&Help"), tr("Show help by default browser."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/help.png")));
		MENU_ENTRY(TeTypes::CMDID_SYSTEM_HELP_VERSION, TeCmdVersion, tr("&Version"), tr("Show version information."), QIcon(new TeAdaptiveIconEngine(":/TableEngine/version.png")));
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

QList<std::pair<QString, TeTypes::CmdId>> TeCommandFactory::groupList()
{
	QList<std::pair<QString, TeTypes::CmdId>> list = {
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

QList<std::pair<QString, TeTypes::CmdId>> TeCommandFactory::custom_groupList()
{
	QList<std::pair<QString, TeTypes::CmdId>> list = {
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

QList<std::pair<QString, TeTypes::CmdId>> TeCommandFactory::static_groupList()
{
	QList<std::pair<QString, TeTypes::CmdId>> list = {
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
