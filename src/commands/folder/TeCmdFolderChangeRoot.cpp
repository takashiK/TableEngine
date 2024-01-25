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

#include "TeCmdFolderChangeRoot.h"

#include "dialogs/TeFilePathDialog.h"
#include "widgets/TeFolderView.h"
#include "TeViewStore.h"
#include "TeUtils.h"

#include <QMessageBox>
#include <QDir>
#include <QDebug>

const char* TeCmdFolderChangeRoot::PARAM_ROOT_PATH = "rootPath";
const char* TeCmdFolderChangeRoot::PARAM_NEW_TAB = "newTab";
const char* TeCmdFolderChangeRoot::PARAM_TAB_POS = "tabPos";


TeCmdFolderChangeRoot::TeCmdFolderChangeRoot()
{
}

TeCmdFolderChangeRoot::~TeCmdFolderChangeRoot()
{
}

bool TeCmdFolderChangeRoot::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdFolderChangeRoot::type()
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

QList<TeMenuParam> TeCmdFolderChangeRoot::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdFolderChangeRoot::execute(TeViewStore* p_store)
{
	QString rootPath;

	bool newTab = cmdParam()->value(PARAM_NEW_TAB, false).toBool();

	int  tabPos = cmdParam()->value(PARAM_TAB_POS, -1).toInt();

	if (tabPos >= p_store->TAB_MAX || tabPos < -1) {
		qDebug() << "TeCmdFolderChangeRoot: Invalid tab position ( " << tabPos << " ).";
		tabPos = -1;
	}

	if (tabPos == -1) {
		tabPos = p_store->currentTabIndex();
	}

	TeFolderView* p_folder = p_store->getFolderView(tabPos);
	if (p_folder && p_folder->getType() != TeTypes::WT_FOLDERVIEW) {
		tabPos = ( tabPos == p_store->TAB_LEFT ? p_store->TAB_RIGHT : p_store->TAB_LEFT );
	}


	if (cmdParam()->contains(PARAM_ROOT_PATH)) {
		rootPath = cmdParam()->value(PARAM_ROOT_PATH).toString();
	}
	else {
		TeFilePathDialog dlg(p_store->mainWindow());
		//initial target is current value.
		TeFolderView* p_folder = p_store->getFolderView(tabPos);
		if (p_folder && p_folder->getType() == TeTypes::WT_FOLDERVIEW) {
			dlg.setCurrentPath(p_folder->currentPath());
		}
		dlg.setWindowTitle(TeFilePathDialog::tr("Change Root path to"));
		if (dlg.exec() == QDialog::Accepted) {
			if (dlg.targetPath().isEmpty()) {
				QMessageBox msg(p_store->mainWindow());
				msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
				msg.setText(QObject::tr("Faild CopyTo Function.\nTarget path is not set."));
				msg.exec();
			}
			else {
				rootPath = dlg.targetPath();
			}
		}
	}

	QDir dir;
	if (dir.exists(rootPath)) {
		p_store->changeRootPath(rootPath, newTab, tabPos);
	}

	return true;
}
