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

#pragma once

#include <QDialog>

/**
 * @file TePasswordDialog.h
 * @brief Declaration of TePasswordDialog.
 * @ingroup dialogs
 */

class QLineEdit;
class QLabel;
class QCheckBox;

/**
 * @class TePasswordDialog
 * @brief Simple modal dialog that prompts the user for an archive password.
 * @ingroup dialogs
 */
class TePasswordDialog : public QDialog
{
	Q_OBJECT

public:
	TePasswordDialog(QWidget* parent = nullptr);
	~TePasswordDialog();

	/** @brief Returns the password entered by the user. */
	QString password() const;

	/** @brief Sets the descriptive prompt (e.g. the archive file name). */
	void setPrompt(const QString& text);

private:
	QLabel*    mp_prompt;
	QLineEdit* mp_password;
	QCheckBox* mp_show;
};
