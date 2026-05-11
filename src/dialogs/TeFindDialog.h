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

#include <QDate>
#include <QDialog>

/**
 * @file TeFindDialog.h
 * @brief Declaration of TeFindDialog.
 * @ingroup dialogs
 */


class QCheckBox;
class QLineEdit;
class QComboBox;
class QPushButton;
class QRadioButton;
class QDateEdit;
class QSpinBox;
class QWidget;

class TeFindDialog : public QDialog
{
	Q_OBJECT

public:
	TeFindDialog(QWidget *parent = nullptr);
	~TeFindDialog();

	enum DateType {
		DateNone,
		DateModified,
		DateCreated
	};
	enum SizeCondition {
		SizeGreaterThan,
		SizeLessThan,
	};
	enum SizeUnit {
		SizeUnit_KB,
		SizeUnit_MB
	};

	bool hasFileNameSearch() const;
	QString searchTerm() const;
	bool isRegex() const;
	bool isCaseSensitive() const;

	bool hasDateSearch() const;
	DateType dateType() const;
	QDate dateFrom() const;
	QDate dateTo() const;

	bool hasSizeSearch() const;
	SizeCondition sizeCondition() const;
	int sizeValue() const;
	SizeUnit sizeUnit() const;


private slots:
	void toggleAdvancedPanel();

private:
	// Search bar
	QLineEdit *m_searchInput;
	QPushButton *m_advancedButton;
	QPushButton *m_searchButton;

	// Advanced panel
	QWidget *m_advancedPanel;

	// File name options
	QRadioButton *m_wildcardRadio;
	QRadioButton *m_regexRadio;
	QCheckBox *m_caseSensitiveCheck;

	// Date options
	QComboBox *m_dateTypeCombo;
	QDateEdit *m_dateFrom;
	QDateEdit *m_dateTo;

	// Size options
	QSpinBox *m_sizeSpinBox;
	QComboBox *m_sizeUnitCombo;
	QComboBox *m_sizeConditionCombo;
};
