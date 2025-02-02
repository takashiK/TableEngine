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

#include "TeCmdEditFavorites.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "TeFavorites.h"

#include "dialogs/TePathListDialog.h"

TeCmdEditFavorites::TeCmdEditFavorites()
{
}

TeCmdEditFavorites::~TeCmdEditFavorites()
{
}

bool TeCmdEditFavorites::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdEditFavorites::type()
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

QList<TeMenuParam> TeCmdEditFavorites::menuParam()
{
	return QList<TeMenuParam>();
}


bool TeCmdEditFavorites::execute(TeViewStore* p_store)
{
	TePathListDialog dlg(p_store->mainWindow());
	TeFavorites favorites;
	dlg.setPathList(favorites.get());
	if (dlg.exec() == QDialog::Accepted) {
		favorites.set(dlg.getPathList());
		favorites.save();
	}

	return true;
}
