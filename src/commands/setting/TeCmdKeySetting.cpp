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

#include "TeCmdKeySetting.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "dialogs/TeKeySetting.h"


TeCmdKeySetting::TeCmdKeySetting()
{
}


TeCmdKeySetting::~TeCmdKeySetting()
{
}

bool TeCmdKeySetting::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdKeySetting::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_TOGGLE
		// TeTypes::CMD_TRIGGER_SELECTION

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_NONE

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_ARCHIVE
	);
}

QList<TeMenuParam> TeCmdKeySetting::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdKeySetting::execute(TeViewStore * p_store)
{
	TeKeySetting dlg(p_store->mainWindow());
	if (dlg.exec() == QDialog::Accepted) {
		p_store->loadKeySetting();
	}
	return true;
}
