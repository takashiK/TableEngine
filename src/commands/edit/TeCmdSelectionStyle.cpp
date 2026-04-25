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

#include "TeCmdSelectionStyle.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

TeCmdSelectionStyle::TeCmdSelectionStyle()
{
}

TeCmdSelectionStyle::~TeCmdSelectionStyle()
{
}

bool TeCmdSelectionStyle::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdSelectionStyle::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	if (!p_cmdParam || !p_cmdParam->contains("SELECTION"))
		return false;

	switch(p_cmdParam->value("SELECTION").toInt()){
	case Explorer:
		return p_store->selectionMode() == TeTypes::SELECTION_EXPLORER;
	case TableEngine:
		return p_store->selectionMode() == TeTypes::SELECTION_TABLE_ENGINE;
	default:
		return false;
	}
}

QFlags<TeTypes::CmdType> TeCmdSelectionStyle::type()
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


bool TeCmdSelectionStyle::execute(TeViewStore* p_store)
{
	if (cmdParam()->contains("SELECTION")) {
		int selection = cmdParam()->value("SELECTION").toInt();
		switch (selection) {
		case Explorer:
			p_store->setSelectionMode(TeTypes::SELECTION_EXPLORER);
			break;
		case TableEngine:
			p_store->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
			break;
		default:
			return true;
		}
	}
	return true;
}
