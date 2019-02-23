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
#include <QtTest/qtest.h>
#include <QApplication>
#include <QMessageBox>
#include <QAbstractButton>

#include "dialogs/TeFilePathDialog.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(TestWidget, test)
{
	QMessageBox msgBox;
	msgBox.setText("The document has been modified.");
	msgBox.setInformativeText("Do you want to save your changes?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Cancel);

	msgBox.setResult(0);
	msgBox.buttons().at(1)->click();
	EXPECT_EQ(msgBox.result(), msgBox.standardButton(msgBox.buttons().at(1)));

	TeFilePathDialog dlg;
	dlg.setCurrentPath("C:/");
	dlg.setWindowTitle(TeFilePathDialog::tr("Copy to"));
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	QApplication a(argc, argv);
	a.setAttribute(Qt::AA_Use96Dpi, true);

	return RUN_ALL_TESTS();
}
