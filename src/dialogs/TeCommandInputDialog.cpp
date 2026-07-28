#include "TeCommandInputDialog.h"
#include "utils/TeToolCommand.h"
#include "TeSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QComboBox>

/**
 * @file TeCommandInputDialog.cpp
 * @brief Declaration of TeCommandInputDialog.
 * @ingroup dialogs
 */


TeCommandInputDialog::TeCommandInputDialog(QWidget *parent)
	: QDialog(parent)
{
	setMinimumWidth(TeSettings::dialogMinimumWidth());
	QVBoxLayout* layout = new QVBoxLayout();

	mp_command = new QLineEdit();
	layout->addWidget(mp_command);

	QHBoxLayout* hlayout = new QHBoxLayout();

	QHBoxLayout* hhlayout = new QHBoxLayout();
	mp_shell = new QCheckBox("shell");
	hhlayout->addWidget(mp_shell);

//	mp_output= new QCheckBox("show output");
//	hlayout->addWidget(mp_output);
	QLabel* label = new QLabel("output:");
	hhlayout->addWidget(label);
	hhlayout->setAlignment(label, Qt::AlignRight | Qt::AlignVCenter);
	hlayout->addLayout(hhlayout);
	mp_output = new QComboBox();
	mp_output->addItem("none", QVariant(TeToolCommand::OUTPUT_NONE));
	mp_output->addItem("stdout", QVariant(TeToolCommand::OUTPUT_STDOUT));
	mp_output->addItem("terminal", QVariant(TeToolCommand::OUTPUT_TERMINAL));
	hlayout->addWidget(mp_output);

	QSettings settings;
	mp_shell->setChecked(settings.value(SETTING_COMMAND_ExecuteWithShell, true).toBool());
	mp_output->setCurrentIndex(mp_output->findData(QVariant(settings.value(SETTING_COMMAND_ExecuteWithTerminal, TeToolCommand::OUTPUT_NONE).toInt())));

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

TeToolCommand::OUTPUT_MODE TeCommandInputDialog::output() const
{
	return static_cast<TeToolCommand::OUTPUT_MODE>(mp_output->currentData().toInt());
}
