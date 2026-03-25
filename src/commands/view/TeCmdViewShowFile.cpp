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

#include "TeCmdViewShowFile.h"
#include "TeUtils.h"
#include "TeViewStore.h"

TeCmdViewShowFile::TeCmdViewShowFile()
{
}

TeCmdViewShowFile::~TeCmdViewShowFile()
{
}

bool TeCmdViewShowFile::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdViewShowFile::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if(p_store && p_cmdParam) {
		ShowFileType type = ShowFileType(p_cmdParam->value("SELECTION").toInt());
		auto current = p_store->fileTypeFlags();
		switch(type) {
			case Hidden:
				return (current & TeTypes::FILETYPE_HIDDEN) == TeTypes::FILETYPE_HIDDEN;
			case System:
				return (current & TeTypes::FILETYPE_SYSTEM) == TeTypes::FILETYPE_SYSTEM;
		}
	}
	return false;
}

QFlags<TeTypes::CmdType> TeCmdViewShowFile::type()
{
	return QFlags<TeTypes::CmdType>(
		// TeTypes::CMD_TRIGGER_NORMAL
		TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}


bool TeCmdViewShowFile::execute(TeViewStore* p_store)
{
	if(p_store && cmdParam()) {
		ShowFileType type = ShowFileType(cmdParam()->value("SELECTION").toInt());
		auto current = p_store->fileTypeFlags();
		switch(type) {
			case Hidden:
				if((current & TeTypes::FILETYPE_HIDDEN) == TeTypes::FILETYPE_HIDDEN) {
					current &= ~TeTypes::FILETYPE_HIDDEN;
				} else {
					current |= TeTypes::FILETYPE_HIDDEN;
				}
				break;
			case System:
				if((current & TeTypes::FILETYPE_SYSTEM) == TeTypes::FILETYPE_SYSTEM) {
					current &= ~TeTypes::FILETYPE_SYSTEM;
				} else {
					current |= TeTypes::FILETYPE_SYSTEM;
				}
				break;
		}
		p_store->setFileTypeFlags(current);
	}
	return true;
}
