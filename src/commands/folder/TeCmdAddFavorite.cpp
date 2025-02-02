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

#include "TeCmdAddFavorite.h"
#include "TeUtils.h"
#include "TeViewStore.h"
#include "dialogs/TeFilePathDialog.h"
#include "TeSettings.h"
#include "TeFavorites.h"
#include "TeHistory.h"
#include "widgets/TeFolderView.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

TeCmdAddFavorite::TeCmdAddFavorite()
{
}

TeCmdAddFavorite::~TeCmdAddFavorite()
{
}

bool TeCmdAddFavorite::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdAddFavorite::type()
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

QList<TeMenuParam> TeCmdAddFavorite::menuParam()
{
	return QList<TeMenuParam>();
}


bool TeCmdAddFavorite::execute(TeViewStore* p_store)
{
	TeFilePathDialog dlg(p_store->mainWindow());
	dlg.setCurrentPath(getCurrentFolder(p_store));
	dlg.setTargetPath(getCurrentFolder(p_store));
	dlg.setWindowTitle(QObject::tr("Add Favorite"));

	TeFavorites favorites;
	dlg.setFavorites(favorites.get());
	dlg.setHistory(p_store->currentFolderView()->getPathHistory());

	if (dlg.exec() == QDialog::Accepted) {
		QString path = dlg.targetPath();
		QFileInfo info(path);
		if (info.exists() && info.isDir()) {
			if (favorites.add(path)) {
				favorites.save();
			}
			else {
				QMessageBox::warning(p_store->mainWindow(), QObject::tr("Add Favorite"), QObject::tr("The maximum number of favorites has been reached."));
			}
		}
		else {
			QMessageBox::warning(p_store->mainWindow(), QObject::tr("Add Favorite"), QObject::tr("The specified folder does not exist."));
		}
	}
	return true;
}
