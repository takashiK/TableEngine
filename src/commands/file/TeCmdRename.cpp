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

#include "TeCmdRename.h"
#include "TeViewStore.h"
#include "TeUtils.h"

#include <QStringList>
#include <QInputDialog>
#include <QFileInfo>

TeCmdRename::TeCmdRename()
{
}

TeCmdRename::~TeCmdRename()
{
}

bool TeCmdRename::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdRename::type()
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

QList<TeMenuParam> TeCmdRename::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdRename::execute(TeViewStore* p_store)
{
	QStringList paths;
	if (getSelectedItemList(p_store, &paths)) {
		for (const auto& path : paths) {
			QFileInfo info(path);
			bool ok;
			QString newName = QInputDialog::getText(p_store->mainWindow(), QObject::tr("Rename"), QObject::tr("New name"), QLineEdit::Normal,  info.fileName(),&ok);
			if (newName.isEmpty() || !ok) {
				return true;
			}
			if (info.fileName() != newName) {
				QFile::rename(path, info.absolutePath() + "/" + newName);
			}
		}
	}
	return true;
}
