#include "TeFilterDialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

TeFilterDialog::TeFilterDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	QHBoxLayout* lineLayout = new QHBoxLayout();

	//LineEdit for filter
	lineLayout->addWidget(new QLabel(tr("Filter:")));
	mp_filter = new QLineEdit();
	lineLayout->addWidget(mp_filter);
	mainLayout->addLayout(lineLayout);

	//Line for case sensitive and regular expression
	lineLayout = new QHBoxLayout();

	//File only
	mp_fileOnly = new QCheckBox(QObject::tr("File only"));
	lineLayout->addWidget(mp_fileOnly);
	
	//Case sensitive
	mp_caseSensitive = new QCheckBox(QObject::tr("Case sensitive"));
	lineLayout->addWidget(mp_caseSensitive);

	//Regular expression
	mp_regexp = new QCheckBox(QObject::tr("Regular expression"));
	lineLayout->addWidget(mp_regexp);

	mainLayout->addLayout(lineLayout);

	//Buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	//Add layouts
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

}

TeFilterDialog::~TeFilterDialog()
{}

QString TeFilterDialog::filter() const
{
	return mp_filter->text();
}

bool TeFilterDialog::fileOnly() const
{
	return mp_fileOnly->isChecked();
}

bool TeFilterDialog::caseSensitive() const
{
	return mp_caseSensitive->isChecked();
}

bool TeFilterDialog::regexp() const
{
	return mp_regexp->isChecked();
}
