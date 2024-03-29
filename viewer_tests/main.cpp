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

#include "viewer/document/TeDocumentSettings.h"
#include "viewer/document/TeDocViewer.h"
#include "viewer/picture/TePictureViewer.h"

#include <QApplication>
#include <QSettings>

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>


#include <QListView>
#include <QFileSystemModel>

#include <gmock/gmock.h>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFileSystemModel model;
	QModelIndex rootIndex = model.setRootPath("C:/");
  

	QListView view;
	view.setModel(&model);
	view.setRootIndex(rootIndex);
	view.setSelectionMode(QAbstractItemView::ExtendedSelection);
	view.setWrapping(true);
	view.setSelectionRectVisible(true);
	view.show();

	return a.exec();

#if 0




	::testing::InitGoogleMock(&argc, argv);

	QApplication::setOrganizationName("TableWare");
	QApplication a(argc, argv);
	a.setAttribute(Qt::AA_Use96Dpi, true);

	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QSettings settings;
	settings.setValue(SETTING_TEXT_HIGHLIGHT_SCHEMA,"viewer/document/schema/text_highlight_schema.json");
	settings.setValue(SETTING_TEXT_HIGHLIGHT_FOLDER, QApplication::applicationDirPath() + "/../highlight");
	settings.setValue(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/../container");

	const QStringList default_codecList = { "UTF-8", "EUC-JP", "Shift_JIS", "ISO-2022-JP" };
	settings.setValue(SETTING_TEXT_CODECS, default_codecList);

	QString str = QApplication::applicationDirPath() + "/../highlight";

	QFileSystemModel model;
	QModelIndex rootIndex = model.setRootPath("C:/");
	QListView view;
	view.setModel(&model);
	view.setRootIndex(rootIndex);
	view.setSelectionMode(QAbstractItemView::ExtendedSelection);
	view.setDragDropMode(QAbstractItemView::DragOnly);
	view.setWrapping(true);
	view.setViewMode(QListView::IconMode);
	view.setSelectionRectVisible(false);
	view.show();

	return a.exec();
#endif // 0

#if 0
	TePictureViewer view;
	//view.open("sample/sample.png");
	view.open("sample/DSC00155.jpg");
	//view.open("../main/Resources/icon/iconmonstr-clipboard-2-64.png");
	//view.open("C:/Users/laffi/Pictures/2019紅葉/DSC00089.jpg");
	view.show();
	return a.exec();
#endif
#if 0
	TeDocViewer view;
	
	view.open("sample/sample.md");
	view.show();

	return a.exec();
#endif
#if 0

	return RUN_ALL_TESTS();
#endif
}
