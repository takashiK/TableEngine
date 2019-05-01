/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "TeAskCreationModeDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QDir>

TeAskCreationModeDialog::TeAskCreationModeDialog(QWidget *parent)
	: QDialog(parent)
{
	//Helpボタン削除
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout();

	QFormLayout* flayout = new  QFormLayout();

	QLabel* label = new QLabel();
	label->setPixmap(QIcon(":/TableEngine/question.png").pixmap(32, 32));
	flayout->addRow(label, new QLabel(tr("Target path not found.")));

	mp_path = new QLineEdit();
	mp_path->setReadOnly(true);
	mp_path->setCursorPosition(0);
	mp_path->setMinimumWidth(300);
	flayout->addRow(tr("Path:"), mp_path);


	mp_createMode = new QButtonGroup();
	QRadioButton* createFolder = new QRadioButton(tr("Create &New folder."),this);
	createFolder->setChecked(true);
	mp_createMode->addButton(createFolder, MODE_CREATE_FOLDER);
	flayout->addRow(tr("Action:"), createFolder);

	QRadioButton* renameFile = new QRadioButton(tr("Copy with &Rename."),this);
	mp_createMode->addButton(renameFile, MODE_RENAME_FILE);
	flayout->addRow("",renameFile);

	layout->addLayout(flayout);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeAskCreationModeDialog::~TeAskCreationModeDialog()
{
}

void TeAskCreationModeDialog::setModeEnabled(CreateMode mode, bool flag)
{
	mp_createMode->button(mode)->setEnabled(flag);
}

void TeAskCreationModeDialog::setCreateMode(CreateMode mode)
{
	mp_createMode->button(mode)->setChecked(true);
}

TeAskCreationModeDialog::CreateMode TeAskCreationModeDialog::createMode()
{
	return CreateMode(mp_createMode->checkedId());
}

void TeAskCreationModeDialog::setTargetPath(const QString & path)
{
	mp_path->setText(path);
}

QString TeAskCreationModeDialog::path()
{
	if (mp_path == nullptr) {
		return QString();
	}
	return QDir::cleanPath(mp_path->text());
}
