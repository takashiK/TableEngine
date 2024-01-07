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

#include "TeCmdExtract.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "widgets/TeFileFolderView.h"
#include "dialogs/TeFilePathDialog.h"
#include "dialogs/TeAskCreationModeDialog.h"

#include "TeArchive.h"

#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>

TeCmdExtract::TeCmdExtract()
{
}


TeCmdExtract::~TeCmdExtract()
{
}

bool TeCmdExtract::isActive( TeViewStore* p_store )
{
	return false;
}

QFlags<TeTypes::CmdType> TeCmdExtract::type()
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

QList<TeMenuParam> TeCmdExtract::menuParam()
{
	return QList<TeMenuParam>();
}

/**
* Copy selected files to target directory
*/
bool TeCmdExtract::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = qobject_cast<TeFileFolderView*>(p_store->currentFolderView());

	if (p_folder != nullptr) {

		QStringList paths;

		if (getSelectedItemList(p_store, &paths)) {
			//get distination folder.
			QStringList extraFlags = {QObject::tr("Create new folder by archive file name.")};

			TeFilePathDialog dlg(p_store->mainWindow(),extraFlags);
			dlg.setExtraFlag(0, true);

			dlg.setCurrentPath(p_folder->currentPath());
			dlg.setWindowTitle(TeFilePathDialog::tr("Extract to"));
			if (dlg.exec() == QDialog::Accepted) {
				if (dlg.targetPath().isEmpty()) {
					QMessageBox msg(p_store->mainWindow());
					msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
					msg.setText(QObject::tr("Faild ExtractTo Function.\nTarget path is not set."));
					msg.exec();
				}
				else {
					extractItems(p_store,paths, dlg.targetPath(),dlg.getExtraFlag(0));
				}
			}
		}
	}

	return true;
}

void TeCmdExtract::extractItems(TeViewStore* p_store, const QStringList & list, const QString & targetPath, bool createArchiveFolder)
{
	QDir dir;

	bool bSuccess = true;

	QFileInfo targetInfo(targetPath);

	//target confirm
	if (targetInfo.exists()) {
		if (targetInfo.isDir()) {
			//Acceptale target
		}
		else {
			//Target is not directory
			bSuccess = false;
		}
	}
	else {
		//directory is not found.
		bSuccess = false;
	}

	if (bSuccess) {
		//start extract
		QProgressDialog progress(QObject::tr(""), QObject::tr("Cancel"), 0, list.count(), p_store->mainWindow());
		progress.setWindowTitle(QObject::tr("Extract"));
		progress.setWindowModality(Qt::WindowModal);


		for (int i = 0; i < list.count(); i++) {
			QString basePath = targetPath;
			QFileInfo info(list[i]);
			if (createArchiveFolder) {
				basePath += QDir::separator() + info.baseName();
			}
			TeArchive::Reader reader(list[i]);

			QString targetInfo = QObject::tr("Extact ") + QString::asprintf("(%d/%d) : ",i,list.count()) + info.fileName() + "\n";

			QObject::connect(&reader, &TeArchive::Reader::maximumValue, &progress, &QProgressDialog::setMaximum);
			QObject::connect(&reader, &TeArchive::Reader::valueChanged, &progress, &QProgressDialog::setValue);
			QObject::connect(&reader, &TeArchive::Reader::currentFileInfoChanged, [&progress,&targetInfo](const TeArchive::FileInfo& info) {
				progress.setLabelText(targetInfo + info.path.right(30));
				});
			QObject::connect(&reader, &TeArchive::Reader::finished, [&progress]() { progress.setValue(progress.maximum()); });
			QObject::connect(&progress, &QProgressDialog::canceled, &reader, &TeArchive::Reader::cancel);

			bool result = reader.extractAll(basePath);
		}
	}

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Extract to following path failed.") + QString("\n") + targetPath);
		msg.exec();
	}
}
