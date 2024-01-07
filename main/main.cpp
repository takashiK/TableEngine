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

#include <TeViewStore.h>
#include <TeDispatcher.h>
#include <commands/TeCommandFactory.h>
#include <platform/platform_util.h>
#include <dialogs/TeOptionSetting.h>
#include <dialogs/TeKeySetting.h>
#include <dialogs/TeMenuSetting.h>

#include <QtWidgets/QApplication>
#include <QTranslator>

#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication::setOrganizationName("TableWare");

	QApplication a(argc, argv);
	//Load translation file.
	QTranslator myappTranslator;
	bool res = myappTranslator.load("tableengine_" + QLocale::system().name());
	if (res)
		a.installTranslator(&myappTranslator);

	//setup setting folder and load settings.
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());
	QSettings::setDefaultFormat(QSettings::IniFormat);
	TeOptionSetting::storeDefaultSettings();
	TeKeySetting::storeDefaultSettings();
	TeMenuSetting::storeDefaultSettings();
	
	//initialize com thread. it use for windows shell.
	threadInitialize();

	//setup dispatcher for command.
	TeDispatcher dispatcher;
	dispatcher.setFactory(TeCommandFactory::factory());

	//create main window
	TeViewStore store;
	store.initialize();
	store.setDispatcher(&dispatcher);
	store.show();

	dispatcher.setViewStore(&store);

	//start event loop.
	return a.exec();
}
