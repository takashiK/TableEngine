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

#include "TeCmdFind.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFindFolderView.h"
#include "utils/TeFinder.h"
#include "utils/TeSearchQuery.h"
#include "dialogs/TeFindDialog.h"
	

TeCmdFind::TeCmdFind()
{
}

TeCmdFind::~TeCmdFind()
{
}

bool TeCmdFind::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdFind::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdFind::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}

bool TeCmdFind::execute(TeViewStore* p_store)
{
	TeFinder* pfinder = p_store->currentFolderView()->makeFinder();
	if (pfinder == nullptr) {
		return true;
	}

	TeFindDialog dlg(p_store->mainWindow());
	if (dlg.exec() != QDialog::Accepted) {
		delete pfinder;
		return true;
	}

	const QString targetPath = p_store->currentFolderView()->currentPath();
	const TeSearchQuery query = TeSearchQuery::fromDialog(dlg, targetPath);
	if (!query.isValid()) {
		delete pfinder;
		return true;
	}

	TeFindFolderView* findView = p_store->findFolderView();
	findView->addSearchEntry(query, pfinder);  // ownership transferred to findView
	p_store->setCurrentFolderView(findView);

	return true;
}
