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
