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

#include "TeCmdViewShowInfo.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

/**
 * @file TeCmdViewShowInfo.cpp
 * @brief Implementation of TeCmdViewShowInfo.
 * @ingroup commands
 */

TeCmdViewShowInfo::TeCmdViewShowInfo()
{
}

TeCmdViewShowInfo::~TeCmdViewShowInfo()
{
}

bool TeCmdViewShowInfo::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdViewShowInfo::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if(p_store && p_cmdParam) {
		ShowInfoType type = ShowInfoType(p_cmdParam->value("SELECTION").toInt());
		switch(type) {
			case FileSize:
				return (p_store->fileInfoFlags() & TeTypes::FILEINFO_SIZE) == TeTypes::FILEINFO_SIZE;
			case ModifiedDate:
				return (p_store->fileInfoFlags() & TeTypes::FILEINFO_MODIFIED) == TeTypes::FILEINFO_MODIFIED;
		}
	}
	return false;
}

QFlags<TeTypes::CmdType> TeCmdViewShowInfo::type()
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


bool TeCmdViewShowInfo::execute(TeViewStore* p_store)
{
	if(p_store && cmdParam()) {
		ShowInfoType type = ShowInfoType(cmdParam()->value("SELECTION").toInt());
		auto current = p_store->fileInfoFlags();
		switch(type) {
			case FileSize:
				if((current & TeTypes::FILEINFO_SIZE) == TeTypes::FILEINFO_SIZE) {
					current &= ~TeTypes::FILEINFO_SIZE;
				} else {
					current |= TeTypes::FILEINFO_SIZE;
				}
				break;
			case ModifiedDate:
				if((current & TeTypes::FILEINFO_MODIFIED) == TeTypes::FILEINFO_MODIFIED) {
					current &= ~TeTypes::FILEINFO_MODIFIED;
				} else {
					current |= TeTypes::FILEINFO_MODIFIED;
				}
				break;
		}
		p_store->setFileInfoFlags(current);
	}
	return true;
}
