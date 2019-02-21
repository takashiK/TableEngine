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
