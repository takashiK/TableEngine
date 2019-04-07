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
