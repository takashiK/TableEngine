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

#include <QApplication>
#include <gmock/gmock.h>

#include <QHBoxLayout>
#include <QDateTime>
#include <widgets/TeArchiveFolderView.h>
#include <widgets/TeFileTreeView.h>

int main(int argc, char *argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	QApplication a(argc, argv);
	a.setAttribute(Qt::AA_Use96Dpi, true);

#if 0
	QWidget* widget = new QWidget;
	QHBoxLayout* layout = new QHBoxLayout;
	TeArchiveFolderView* view = new TeArchiveFolderView;
	view->addDirEntry("test/test1");
	view->addDirEntry("test/test2");
	view->addDirEntry("test/test3");
	view->addEntry("test/file1.zip", 10000, QDateTime(), "test");
	view->addEntry("test/file2.7", 1000000000, QDateTime(), "test");
	view->addEntry("test/test2/file3.", 10, QDateTime(), "test");
	//view->tree()->setHeaderHidden(false);
	view->setRootPath("C:/Projects/vs2015/QtArchiveTest/QtArchiveTest/docs.zip");
	view->tree()->expandAll();
//	view->setCurrentPath("test/test2");

	layout->addWidget(view);
	layout->insertWidget(0,view->tree());
	widget->setLayout(layout);
	widget->show();

	a.exec();

	delete view;
	delete widget;
	return 0;
#endif
	return RUN_ALL_TESTS();
}
