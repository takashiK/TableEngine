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

#include "TeCmdDivideFile.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "dialogs/TeDivideDialog.h"

#include <QDir>
#include <QMessageBox>

TeCmdDivideFile::TeCmdDivideFile()
{
}

TeCmdDivideFile::~TeCmdDivideFile()
{
}

bool TeCmdDivideFile::isActive(TeViewStore* p_store)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdDivideFile::type()
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

QList<TeMenuParam> TeCmdDivideFile::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdDivideFile::execute(TeViewStore* p_store)
{
	QString filepath = getCurrentItem(p_store);
	if (!filepath.isEmpty()) {
		TeDivideDialog dialog;
		dialog.setFilepath(filepath);
		if (dialog.exec() == QDialog::Accepted) {
			QFileInfo srcInfo(filepath);
			QDir dir(srcInfo.absolutePath());

			QString outPath = dir.absoluteFilePath(dialog.outPath());
			qsizetype divSize = dialog.size();

			QFileInfo info(outPath);
			if (info.isFile()) {
				QMessageBox::warning(
					p_store->mainWindow(),
					QObject::tr("File already exists"),
					QObject::tr("File already exists: %1").arg(outPath)
				);
				return true;
			}

			if( !info.exists() ){
				// confirm to create directory
				if( QMessageBox::question(
					p_store->mainWindow(),
					QObject::tr("Create directory"),
					QObject::tr("Directory does not exist. \nCreate directory: %1").arg(info.absolutePath()),
					QMessageBox::Yes | QMessageBox::No
				) != QMessageBox::Yes ){
					return true;
				}
				
				if( !dir.mkpath(info.absolutePath()) ){
					QMessageBox::warning(
						p_store->mainWindow(),
						QObject::tr("Failed to create directory"),
						QObject::tr("Failed to create directory:\n %1").arg(info.absolutePath())
					);
					return true;
				}
			}
			
			// make divided files into outPath.
			QFile srcFile(filepath);
			if( !srcFile.open(QIODevice::ReadOnly) ){
				QMessageBox::warning(
					p_store->mainWindow(),
					QObject::tr("Failed to open file"),
					QObject::tr("Failed to open file:\n %1").arg(filepath)
				);
				return true;
			}
			int count = 0;
			const qsizetype copySize = 1024 * 1024 * 16; // 16MB
			while (srcFile.bytesAvailable()) {
				QFile dstFile(outPath + "/" + srcInfo.fileName() + QString(".%1").arg(count, 3, 10, QChar('0')));
				if( !dstFile.open(QIODevice::WriteOnly) ){
					QMessageBox::warning(
						p_store->mainWindow(),
						QObject::tr("Failed to open file"),
						QObject::tr("Failed to open file:\n %1").arg(dstFile.fileName())
					);
					return true;
				}
				qsizetype remSize = divSize;
				while(srcFile.bytesAvailable()){
					if(remSize < copySize){
						dstFile.write(srcFile.read(remSize));
						break;
					}
					else {
						dstFile.write(srcFile.read(copySize));
						remSize -= copySize;
					}
				}
				dstFile.close();
				count++;
			}
		}
	}
	return true;
}
