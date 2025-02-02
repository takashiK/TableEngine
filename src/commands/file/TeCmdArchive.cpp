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

#include "TeCmdArchive.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "widgets/TeFileFolderView.h"
#include "TeArchive.h"


#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>

TeCmdArchive::TeCmdArchive()
{
}


TeCmdArchive::~TeCmdArchive()
{
}

bool TeCmdArchive::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdArchive::type()
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

QList<TeMenuParam> TeCmdArchive::menuParam()
{
	return QList<TeMenuParam>();
}

/**
* Copy selected files to target directory
*/
bool TeCmdArchive::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

	if (p_folder != nullptr) {

		QStringList paths;

		if (getSelectedItemList(p_store, &paths)) {
			//get distination folder.
			QString targetPath = QFileDialog::getSaveFileName(p_store->mainWindow(), QObject::tr("Archive file path"), p_folder->currentPath(), QObject::tr("Archive (*.zip *.tgz *.tar.gz *.tar.bz2 *.7z)"));
			if (targetPath.isEmpty()) {
				QMessageBox msg(p_store->mainWindow());
				msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
				msg.setText(QObject::tr("Faild Archive Function.\nArchive file path is not set."));
				msg.exec();
			}
			else {
				archiveItems(p_store,paths, targetPath,p_folder->currentPath());
			}
		}
	}

	return true;
}

void TeCmdArchive::archiveItems(TeViewStore* p_store, const QStringList & list, const QString & targetPath, const QString& currentPath)
{
	QDir dir;

	bool bSuccess = true;
	QFileInfo info(targetPath);

	TeArchive::ArchiveType type = TeArchive::AR_NONE;
	QFileInfo targetInfo(targetPath);
	if (targetInfo.completeSuffix() == "zip") {
		type = TeArchive::AR_ZIP;
	}
	else if (targetInfo.completeSuffix() == "tgz") {
		type = TeArchive::AR_TAR_GZIP;
	}
	else if (targetInfo.completeSuffix() == "tar.gz") {
		type = TeArchive::AR_TAR_GZIP;
	}
	else if (targetInfo.completeSuffix() == "tar.bz2") {
		type = TeArchive::AR_TAR_BZIP2;
	}
	else if (targetInfo.completeSuffix() == "7z") {
		type = TeArchive::AR_7ZIP;
	}

	if (type == TeArchive::AR_NONE) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Archive type is not supported.") + QString("\n") + targetPath);
		msg.exec();
		bSuccess = false;
	}

	if (bSuccess) {
		TeArchive::Writer writer;
		QString currentPathWithSlash = currentPath.isEmpty() ? "" : currentPath + "/";

		QProgressDialog progress(QObject::tr(""), QObject::tr("Cancel"), 0, list.count(), p_store->mainWindow());
		progress.setWindowTitle(QObject::tr("Archive"));
/*
		QObject::connect(&writer, &TeArchive::Writer::addedFileInfo, [&progress,&writer](const TeArchive::FileInfo& info){
				progress.setLabelText(QObject::tr("Add : ") + info.path.right(30));
			});
*/
		progress.setWindowModality(Qt::WindowModal);

		progress.setMaximum(list.count());
		for (const auto& path: list) {
			if (path.startsWith(currentPathWithSlash)) {
				bSuccess &= writer.addEntry(path, path.sliced(currentPathWithSlash.length()));
			}
			else {
				bSuccess = false;
				break;
			}
		}
		if (bSuccess) {
			QObject::connect(&writer, &TeArchive::Writer::maximumValue, &progress, &QProgressDialog::setMaximum);
			QObject::connect(&writer, &TeArchive::Writer::valueChanged, &progress, &QProgressDialog::setValue);
			QObject::connect(&writer, &TeArchive::Writer::currentFileInfoChanged, [&progress](const TeFileInfo& info) {
					progress.setLabelText(QObject::tr("Archive : ") + info.path.right(30));
				});
			QObject::connect(&writer, &TeArchive::Writer::finished, [&progress]() { progress.setValue(progress.maximum()); });
			QObject::connect(&progress, &QProgressDialog::canceled, &writer, &TeArchive::Writer::cancel);

			bSuccess = writer.archive(targetPath,type);
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Archive to following path failed.") + QString("\n") + targetPath);
		msg.exec();
	}
}
