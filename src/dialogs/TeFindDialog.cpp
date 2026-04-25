#include "TeFindDialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QDateEdit>
#include <QSpinBox>
#include <QButtonGroup>

TeFindDialog::TeFindDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Find"));
	setMinimumWidth(550);

	// Main layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(15, 15, 15, 15);
	mainLayout->setSpacing(12);

	// === Search bar ===
	QHBoxLayout *searchBarLayout = new QHBoxLayout();
	searchBarLayout->setSpacing(8);

	m_searchInput = new QLineEdit(this);
	m_searchInput->setPlaceholderText(tr("Enter search term (e.g. *.txt)"));
	searchBarLayout->addWidget(m_searchInput, 1);

	m_advancedButton = new QPushButton(tr("Advanced ∨"), this);
	connect(m_advancedButton, &QPushButton::clicked, this, &TeFindDialog::toggleAdvancedPanel);
	searchBarLayout->addWidget(m_advancedButton);

	m_searchButton = new QPushButton(tr("Search"), this);
	searchBarLayout->addWidget(m_searchButton);

	mainLayout->addLayout(searchBarLayout);

	// === Advanced panel (collapsible) ===
	m_advancedPanel = new QWidget(this);
	m_advancedPanel->setObjectName("advancedPanel");
	m_advancedPanel->setVisible(false);

	QVBoxLayout *advancedLayout = new QVBoxLayout(m_advancedPanel);
	advancedLayout->setContentsMargins(15, 15, 15, 15);
	advancedLayout->setSpacing(12);

	// --- File name options ---
	QHBoxLayout *fileNameLayout = new QHBoxLayout();
	fileNameLayout->setSpacing(15);

	QLabel *fileNameLabel = new QLabel(tr("Filename:"), this);
	fileNameLabel->setFixedWidth(80);
	fileNameLabel->setStyleSheet("font-weight: bold;");
	fileNameLayout->addWidget(fileNameLabel);

	m_wildcardRadio = new QRadioButton(tr("Wildcard"), this);
	m_wildcardRadio->setChecked(true);
	fileNameLayout->addWidget(m_wildcardRadio);

	m_regexRadio = new QRadioButton(tr("Regex"), this);
	fileNameLayout->addWidget(m_regexRadio);

	QButtonGroup *matchGroup = new QButtonGroup(this);
	matchGroup->addButton(m_wildcardRadio);
	matchGroup->addButton(m_regexRadio);

	m_caseSensitiveCheck = new QCheckBox(tr("Case Sensitive"), this);
	fileNameLayout->addWidget(m_caseSensitiveCheck);

	fileNameLayout->addStretch();
	advancedLayout->addLayout(fileNameLayout);

	// --- Date options ---
	QHBoxLayout *dateRow1Layout = new QHBoxLayout();
	dateRow1Layout->setSpacing(15);

	QLabel *dateLabel = new QLabel(tr("Date:"), this);
	dateLabel->setFixedWidth(80);
	dateLabel->setStyleSheet("font-weight: bold;");
	dateRow1Layout->addWidget(dateLabel);

	m_dateTypeCombo = new QComboBox(this);
	m_dateTypeCombo->addItems({tr("None"), tr("Modified"), tr("Created")});
	dateRow1Layout->addWidget(m_dateTypeCombo);

	dateRow1Layout->addStretch();
	advancedLayout->addLayout(dateRow1Layout);

	QHBoxLayout *dateRow2Layout = new QHBoxLayout();
	dateRow2Layout->setSpacing(8);

	QLabel *datePlaceholder = new QLabel(this);
	datePlaceholder->setFixedWidth(80);
	dateRow2Layout->addWidget(datePlaceholder);

	m_dateFrom = new QDateEdit(this);
	m_dateFrom->setCalendarPopup(true);
	dateRow2Layout->addWidget(m_dateFrom);

	QLabel *tildeLabel = new QLabel(tr("to"), this);
	dateRow2Layout->addWidget(tildeLabel);

	m_dateTo = new QDateEdit(this);
	m_dateTo->setCalendarPopup(true);
	dateRow2Layout->addWidget(m_dateTo);

	dateRow2Layout->addStretch();
	advancedLayout->addLayout(dateRow2Layout);

	// --- Size options ---
	QHBoxLayout *sizeLayout = new QHBoxLayout();
	sizeLayout->setSpacing(15);

	QLabel *sizeLabel = new QLabel(tr("Size:"), this);
	sizeLabel->setFixedWidth(80);
	sizeLabel->setStyleSheet("font-weight: bold;");
	sizeLayout->addWidget(sizeLabel);

	m_sizeSpinBox = new QSpinBox(this);
	m_sizeSpinBox->setMinimum(0);
	m_sizeSpinBox->setMaximum(999999);
	m_sizeSpinBox->setFixedWidth(80);
	sizeLayout->addWidget(m_sizeSpinBox);

	m_sizeUnitCombo = new QComboBox(this);
	m_sizeUnitCombo->addItems({"KB", "MB"});
	sizeLayout->addWidget(m_sizeUnitCombo);

	m_sizeConditionCombo = new QComboBox(this);
	m_sizeConditionCombo->addItems({tr("or more"), tr("or less")});
	sizeLayout->addWidget(m_sizeConditionCombo);

	sizeLayout->addStretch();
	advancedLayout->addLayout(sizeLayout);

	mainLayout->addWidget(m_advancedPanel);

	// Add stretch to push content to top
	mainLayout->addStretch();
}

TeFindDialog::~TeFindDialog()
{}

bool TeFindDialog::hasFileNameSearch() const
{
	return !m_searchInput->text().isEmpty();
}

QString TeFindDialog::searchTerm() const
{
	return m_searchInput->text();
}

bool TeFindDialog::isRegex() const
{
	return m_regexRadio->isChecked();
}

bool TeFindDialog::isCaseSensitive() const
{
	return m_caseSensitiveCheck->isChecked();
}

bool TeFindDialog::hasDateSearch() const
{
	return dateType() != DateNone;
}

TeFindDialog::DateType TeFindDialog::dateType() const
{
	switch (m_dateTypeCombo->currentIndex()) {
	case 1:  return DateModified;
	case 2:  return DateCreated;
	default: return DateNone;
	}
}

QDate TeFindDialog::dateFrom() const
{
	return m_dateFrom->date();
}

QDate TeFindDialog::dateTo() const
{
	return m_dateTo->date();
}

bool TeFindDialog::hasSizeSearch() const
{
	return m_sizeSpinBox->value() > 0;
}

TeFindDialog::SizeCondition TeFindDialog::sizeCondition() const
{
	return m_sizeConditionCombo->currentIndex() == 0 ? SizeGreaterThan : SizeLessThan;
}

int TeFindDialog::sizeValue() const
{
	return m_sizeSpinBox->value();
}

TeFindDialog::SizeUnit TeFindDialog::sizeUnit() const
{
	return m_sizeUnitCombo->currentIndex() == 0 ? SizeUnit_KB : SizeUnit_MB;
}

void TeFindDialog::toggleAdvancedPanel()
{
	bool isVisible = m_advancedPanel->isVisible();
	m_advancedPanel->setVisible(!isVisible);
	m_advancedButton->setText(isVisible ? tr("Advanced ∨") : tr("Advanced ∧"));

	if (isVisible) {
		// Shrink dialog when panel is closed
		adjustSize();
	}
}
