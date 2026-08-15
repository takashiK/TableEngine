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
#include <utils/TeAppPaths.h>
#include <dialogs/TeOptionSetting.h>
#include <dialogs/TeKeySetting.h>
#include <dialogs/TeMenuSetting.h>
#include <dialogs/TeToolbarSetting.h>
#include <dialogs/TeEditToolSetting.h>
#include <viewer/document/TeDocumentSettings.h>

#include "version.h"

#include <QtWidgets/QApplication>
#include <QLocale>
#include <QTranslator>

#include <QPixmapCache>

#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication::setOrganizationName("TableWare");
	QApplication::setApplicationName("TableEngine");
	QApplication::setApplicationVersion(APP_VERSION_STR);
	QApplication a(argc, argv);

	//Load translation file.
	QTranslator myappTranslator;
	const QString languageCode = QLocale::system().name().section('_', 0, 0);
	bool res = myappTranslator.load(":/i18n/tableengine_" + languageCode + ".qm");
	if (res)
		a.installTranslator(&myappTranslator);

	//setup setting folder and load settings.
#ifdef Q_OS_WIN
	// Portable layout: keep the ini file next to the executable.
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());
#endif
	// On Linux the default IniFormat/UserScope path already resolves under
	// QStandardPaths::AppConfigLocation (~/.config/<org>/<app>.ini).
	QSettings::setDefaultFormat(QSettings::IniFormat);
	TeOptionSetting::storeDefaultSettings();
	TeKeySetting::storeDefaultSettings();
	TeMenuSetting::storeDefaultSettings();
	TeToolbarSetting::storeDefaultSettings();
	TeEditToolSetting::storeDefaultSettings();

	TeCommandFactory::factory()->loadUserCommands();

	//set document viewer
	QSettings settings;
	settings.setValue(SETTING_TEXT_HIGHLIGHT_SCHEMA,":/Schema/text_highlight.json");
	settings.setValue(SETTING_TEXT_HIGHLIGHT_FOLDER, teUserAssetDir() + "/highlight");

	QPixmapCache::setCacheLimit(51200); // 50MB

	//setup dispatcher for command.
	TeDispatcher dispatcher;
	dispatcher.setFactory(TeCommandFactory::factory());


	//initialize com thread. it use for windows shell.
	threadInitialize(&a);


	//create main window
	TeViewStore store;
	store.initialize();
	store.setDispatcher(&dispatcher);
	store.show();

	dispatcher.setViewStore(&store);

	//start event loop.
	return a.exec();
}
