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

#include "TeCmdDelete.h"
#include "TeViewStore.h"
#include "TeUtils.h"
#include "platform/platform_util.h"

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>



TeCmdDelete::TeCmdDelete()
{
}


TeCmdDelete::~TeCmdDelete()
{
}

bool TeCmdDelete::isAvailable()
{
	return true;
}

/**
* Delete selected files.
*/
bool TeCmdDelete::execute(TeViewStore* p_store)
{
	QStringList paths;

	if (getSelectedItemList(p_store, &paths)) {
		//delete target files.
		deleteItems(p_store, paths);
	}

	return true;
}

void TeCmdDelete::deleteItems(TeViewStore* p_store, const QStringList & list)
{
	QDir dir;

	bool bSuccess = true;

	bSuccess = deleteFiles(list);

	if (!bSuccess) {
		QMessageBox msg(p_store->mainWindow());
		msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
		msg.setText(QObject::tr("Failed delete files."));
		msg.exec();
	}
}