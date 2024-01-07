#include "TeCommandInputDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>

TeCommandInputDialog::TeCommandInputDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();

	mp_command = new QLineEdit();
	layout->addWidget(mp_command);

	QHBoxLayout* hlayout = new QHBoxLayout();
	mp_shell = new QCheckBox("shell");
	hlayout->addWidget(mp_shell);
	mp_output= new QCheckBox("show output");
	hlayout->addWidget(mp_output);

	layout->addLayout(hlayout);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);


	setLayout(layout);
}

TeCommandInputDialog::~TeCommandInputDialog()
{}

QString TeCommandInputDialog::command() const
{
	return mp_command->text();
}

bool TeCommandInputDialog::shell() const
{
	return mp_shell->isChecked();
}

bool TeCommandInputDialog::output() const
{
	return mp_output->isChecked();
}
