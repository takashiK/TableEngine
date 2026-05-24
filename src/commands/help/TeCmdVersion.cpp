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

#include "TeCmdVersion.h"
#include "utils/TeUtils.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPixmap>

/**
 * @file TeCmdVersion.cpp
 * @brief Declaration of TeCmdVersion.
 * @ingroup commands
 */


TeCmdVersion::TeCmdVersion()
{
}

TeCmdVersion::~TeCmdVersion()
{
}

bool TeCmdVersion::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdVersion::type()
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


bool TeCmdVersion::execute(TeViewStore* p_store)
{
	NOT_USED(p_store);
	QMessageBox msgBox;
	msgBox.setWindowTitle(QCoreApplication::tr("About %1").arg(QCoreApplication::applicationName()));
	msgBox.setText(
		QString("<b>%1</b> &nbsp; version %2<br><br>"
		        "A lightweight file explorer.<br><br>"
		        "Copyright (C) 2019 Takashi Kuwabara")
			.arg(QCoreApplication::applicationName())
			.arg(QCoreApplication::applicationVersion())
	);
	msgBox.setIconPixmap(
		QPixmap(":/TableEngine/application.ico").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
	);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.exec();
	return true;
}
