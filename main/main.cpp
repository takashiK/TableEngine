/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
	myappTranslator.load("tableengine_" + QLocale::system().name());
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
	store.setDispatcher(&dispatcher); //ViewをDispatcherに接続
	store.show();

	dispatcher.setViewStore(&store);

	//start event loop.
	return a.exec();
}
