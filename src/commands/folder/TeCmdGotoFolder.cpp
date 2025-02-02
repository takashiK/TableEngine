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

#include "TeCmdGotoFolder.h"
#include "TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "dialogs/TeFilePathDialog.h"

#include <QDir>
#include <QMessageBox>

TeCmdGotoFolder::TeCmdGotoFolder()
{
}

TeCmdGotoFolder::~TeCmdGotoFolder()
{
}

bool TeCmdGotoFolder::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdGotoFolder::type()
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

QList<TeMenuParam> TeCmdGotoFolder::menuParam()
{
	return QList<TeMenuParam>();
}


bool TeCmdGotoFolder::execute(TeViewStore* p_store)
{
	TeFilePathDialog dlg(p_store->mainWindow());
	dlg.setCurrentPath(getCurrentFolder(p_store));
	if(dlg.exec() == QDialog::Accepted)
	{
		QString path = dlg.targetPath();
		QDir dir(getCurrentFolder(p_store));
		if(!path.isEmpty() && dir.exists(path))
		{
			path = dir.absoluteFilePath(path);
			TeFolderView* p_view = p_store->currentFolderView();
			QString rootPath = dir.absoluteFilePath(p_view->rootPath());

			if(path.startsWith(rootPath))
			{
				p_view->setCurrentPath(path);
			}
			else {
				QDir rootDir(path);
				while (!rootDir.isRoot()) {
					rootDir.cdUp();
				}
				p_view->setRootPath(rootDir.absolutePath());
				p_view->setCurrentPath(path);
			}

		}
		else {
			QMessageBox::warning(p_store->mainWindow(), QObject::tr("Error"), QObject::tr("The specified folder does not exist."));
		}
	}
	return true;
}
