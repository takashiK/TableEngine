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

#include "TeCmdViewFileOrderBy.h"
#include "TeUtils.h"
#include "TeViewStore.h"

TeCmdViewFileOrderBy::TeCmdViewFileOrderBy()
{
}

TeCmdViewFileOrderBy::~TeCmdViewFileOrderBy()
{
}

bool TeCmdViewFileOrderBy::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdViewFileOrderBy::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if(p_store && p_cmdParam) {
		OrderByType order = OrderByType(p_cmdParam->value("SELECTION").toInt());
		switch(order) {
			case FileName:
				return p_store->fileOrderBy() == TeTypes::ORDER_NAME;
			case FileSize:
				return p_store->fileOrderBy() == TeTypes::ORDER_SIZE;
			case Extension:
				return p_store->fileOrderBy() == TeTypes::ORDER_EXTENSION;
			case ModifiedDate:
				return p_store->fileOrderBy() == TeTypes::ORDER_MODIFIED;
		}
	}
	return false;
}

QFlags<TeTypes::CmdType> TeCmdViewFileOrderBy::type()
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


bool TeCmdViewFileOrderBy::execute(TeViewStore* p_store)
{
	if(p_store && cmdParam()) {
		OrderByType order = OrderByType(cmdParam()->value("SELECTION").toInt());
		switch(order) {
			case FileName:
				p_store->setFileOrderBy(TeTypes::ORDER_NAME);
				break;
			case FileSize:
				p_store->setFileOrderBy(TeTypes::ORDER_SIZE);
				break;
			case Extension:
				p_store->setFileOrderBy(TeTypes::ORDER_EXTENSION);
				break;
			case ModifiedDate:
				p_store->setFileOrderBy(TeTypes::ORDER_MODIFIED);
				break;
		}
	}
	return true;
}
