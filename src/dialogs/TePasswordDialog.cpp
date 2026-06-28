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

#include "TePasswordDialog.h"
#include "TeSettings.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

/**
 * @file TePasswordDialog.cpp
 * @brief Implementation of TePasswordDialog.
 * @ingroup dialogs
 */

TePasswordDialog::TePasswordDialog(QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Password"));
	setMinimumWidth(TeSettings::dialogMinimumWidth());

	QVBoxLayout* layout = new QVBoxLayout();

	mp_prompt = new QLabel(tr("Enter the password for the archive:"));
	mp_prompt->setWordWrap(true);
	layout->addWidget(mp_prompt);

	mp_password = new QLineEdit();
	mp_password->setEchoMode(QLineEdit::Password);
	layout->addWidget(mp_password);

	mp_show = new QCheckBox(tr("Show password"));
	connect(mp_show, &QCheckBox::toggled, this, [this](bool checked) {
		mp_password->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
	});
	layout->addWidget(mp_show);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);

	mp_password->setFocus();
}

TePasswordDialog::~TePasswordDialog()
{}

QString TePasswordDialog::password() const
{
	return mp_password->text();
}

void TePasswordDialog::setPrompt(const QString& text)
{
	mp_prompt->setText(text);
}
