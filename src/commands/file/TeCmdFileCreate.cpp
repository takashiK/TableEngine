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

#include "TeCmdFileCreate.h"
#include "TeUtils.h"
#include "TeViewStore.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>

TeCmdFileCreate::TeCmdFileCreate()
{
}

TeCmdFileCreate::~TeCmdFileCreate()
{
}

bool TeCmdFileCreate::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdFileCreate::type()
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

QList<TeMenuParam> TeCmdFileCreate::menuParam()
{
	return QList<TeMenuParam>();
}


bool TeCmdFileCreate::execute(TeViewStore* p_store)
{
	//Ask the user for the file name.
	QString fileName = QInputDialog::getText(p_store->mainWindow(), QObject::tr("Create File"), QObject::tr("Enter the file name:"));

	QDir currentDir(getCurrentFolder(p_store));
	QFileInfo fileInfo(currentDir,fileName);
	if(fileInfo.absolutePath() != getCurrentFolder(p_store))
	{
		QMessageBox::warning(p_store->mainWindow(), QObject::tr("Error"), QObject::tr("The file must be created in the current folder."));
		return true;
	}
	if (fileInfo.exists())
	{
		QMessageBox::warning(p_store->mainWindow(), QObject::tr("Error"), QObject::tr("The file already exists."));
		return true;
	}
	QFile file(fileInfo.absoluteFilePath());
	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(p_store->mainWindow(), QObject::tr("Error"), QObject::tr("Failed to create the file."));
		return true;
	}
	file.close();
	return true;
}
