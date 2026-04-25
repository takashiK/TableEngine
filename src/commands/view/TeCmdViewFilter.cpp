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

#include "TeCmdViewFilter.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFolderView.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileSortProxyModel.h"

#include "dialogs/TeFilterDialog.h"

#include <QRegularExpression>

TeCmdViewFilter::TeCmdViewFilter()
{
}

TeCmdViewFilter::~TeCmdViewFilter()
{
}

bool TeCmdViewFilter::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdViewFilter::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdViewFilter::type()
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


bool TeCmdViewFilter::execute(TeViewStore* p_store)
{
	TeFilterDialog dialog(p_store->mainWindow());
	dialog.setForceFileOnly(true);
	if(dialog.exec() == QDialog::Accepted) {
		QString filter = dialog.filter();
		bool fileOnly = dialog.fileOnly();
		bool caseSensitive = dialog.caseSensitive();
		bool regexp = dialog.regexp();

		TeFileListView* p_list = p_store->currentFolderView()->list();
		TeFileSortProxyModel* proxyModel = qobject_cast<TeFileSortProxyModel*>(p_list->model());

		if (proxyModel) {
			QRegularExpression re;
			if (filter.isEmpty()){
			} else if (regexp) {
				if(!filter.startsWith("^")){
					re.setPattern("^" + filter);
				}else{
					re.setPattern(filter);
				}
				re.setPatternOptions(caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
			}
			else {
				QRegularExpression tmp = QRegularExpression::fromWildcard(filter, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
				re.swap(tmp);
			}
			proxyModel->setFileRegex(re);
		}
	}
	
	return true;
}
