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

#include "TeCmdSelectFilter.h"
#include "TeViewStore.h"
#include "TeUtils.h"

#include "dialogs/TeFilterDialog.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"

#include <QItemSelectionModel>
#include <QRegularexpression>

TeCmdSelectFilter::TeCmdSelectFilter()
{
}

TeCmdSelectFilter::~TeCmdSelectFilter()
{
}

bool TeCmdSelectFilter::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdSelectFilter::type()
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

QList<TeMenuParam> TeCmdSelectFilter::menuParam()
{
	return QList<TeMenuParam>();
}


bool TeCmdSelectFilter::execute(TeViewStore* p_store)
{
	TeFilterDialog dialog(p_store->mainWindow());
	if(dialog.exec() == QDialog::Accepted) {
		QString filter = dialog.filter();
		bool fileOnly = dialog.fileOnly();
		bool caseSensitive = dialog.caseSensitive();
		bool regexp = dialog.regexp();

		TeFileListView* p_list = p_store->currentFolderView()->list();
		QItemSelectionModel* selectionModel = p_list->selectionModel();
		selectionModel->clear();

		QRegularExpression re;
		if (regexp) {
			re.setPattern(filter);
			re.setPatternOptions(caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
		}
		else {
			QRegularExpression tmp = QRegularExpression::fromWildcard(filter, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			re.swap(tmp);
		}

		QModelIndex root = p_list->rootIndex();
		for (int i = 0; i < p_list->model()->rowCount(root); i++) {
			QModelIndex index = p_list->model()->index(i, 0, root);
			QString name = p_list->model()->data(index, Qt::DisplayRole).toString();
			if (!fileOnly || !isDir(p_list->model(), index)) {
				if (re.match(name).hasMatch()) {
					selectionModel->select(index, QItemSelectionModel::Select);
				}
			}
		}
	}
	return true;
}
