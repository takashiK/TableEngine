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

#include "TeCmdRenameMulti.h"
#include "TeUtils.h"
#include "TeViewStore.h"
#include "dialogs/TeRenameMultiDialog.h"

#include <QStringList>
#include <QFileInfo>
#include <QFile>

TeCmdRenameMulti::TeCmdRenameMulti()
{
}

TeCmdRenameMulti::~TeCmdRenameMulti()
{
}

bool TeCmdRenameMulti::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdRenameMulti::type()
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

QList<TeMenuParam> TeCmdRenameMulti::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdRenameMulti::execute(TeViewStore* p_store)
{
	QStringList files;
	if (getSelectedItemList(p_store, &files)) {
		TeRenameMultiDialog dialog;
		if (dialog.exec() == QDialog::Accepted) {
			QString baseName = dialog.baseName();
			QString extention = dialog.extention();
			int digits = dialog.digits();
			int startNum = dialog.startNum();
			int order = dialog.order();

			//conveert files to fileInfo list
			QList<QFileInfo> fileInfoList;
			for (const QString& file : files) {
				fileInfoList.append(QFileInfo(file));
			}

			//sort by order
			if (order == TeRenameMultiDialog::ORDER_FILENAME) {
				std::sort(fileInfoList.begin(), fileInfoList.end(), [](const QFileInfo& a, const QFileInfo& b) {
					return a.fileName() < b.fileName();
				});
			}
			else {
				std::sort(fileInfoList.begin(), fileInfoList.end(), [](const QFileInfo& a, const QFileInfo& b) {
					return a.birthTime() < b.birthTime();
				});
			}

			//rename
			for (int i = 0; i < files.size(); i++) {
				QString newName = baseName;
				newName += QString("%1").arg(startNum + i, digits, 10, QChar('0'));
				newName += extention;
				QFile::rename(fileInfoList[i].filePath(), fileInfoList[i].absolutePath() + "/" +  newName);
			}
		
		}
	}

	return true;
}
