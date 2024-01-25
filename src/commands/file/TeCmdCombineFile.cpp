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

#include "TeCmdCombineFile.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "dialogs/TeCombineDialog.h"

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

TeCmdCombineFile::TeCmdCombineFile()
{
}

TeCmdCombineFile::~TeCmdCombineFile()
{
}

bool TeCmdCombineFile::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdCombineFile::type()
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

QList<TeMenuParam> TeCmdCombineFile::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdCombineFile::execute(TeViewStore* p_store)
{
	QStringList paths;
	if (getSelectedItemList(p_store, &paths)) {
		TeCombineDialog dialog;
		QString currentFolder = getCurrentFolder(p_store);
		dialog.setBasePath(currentFolder);
		dialog.setInputFiles(paths);
		if (dialog.exec() == QDialog::Accepted) {
			QStringList inputFiles = dialog.inputFiles();
			QDir dir(currentFolder);
			QFile outFile( dir.absoluteFilePath(dialog.outputFile()) );
			if (outFile.exists()) {
				//confirm overwrite
				if (QMessageBox::question(
					nullptr,
					QObject::tr("Overwrite"),
					QObject::tr("File already exists. Overwrite?"),
					QMessageBox::Yes | QMessageBox::No
				) != QMessageBox::Yes) {
					return true;
				}
			}
			//combine files
			if (outFile.open(QIODevice::WriteOnly | QIODeviceBase::Truncate)) {
				for (const auto& path : inputFiles) {
					QFile inputFile( path );
					if (inputFile.open(QIODevice::ReadOnly)) {
						while (inputFile.bytesAvailable()) {
							outFile.write(inputFile.read(16*1024*1024));
						}
						inputFile.close();
					}
				}
			}
		}
	}
	return true;
}
