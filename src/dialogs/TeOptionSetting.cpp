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
#include "TeOptionSetting.h"
#include "TeSettings.h"
#include "TeSelectPathDialog.h"

#include <QSettings>
#include <QDir>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>

TeOptionSetting::TeOptionSetting(QWidget *parent)
	: QDialog(parent)
{
	//Helpボタン削除
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(tr("Option"));
	setMinimumWidth(380);

	QVBoxLayout* layout = new QVBoxLayout();

	QTabWidget* tab = new QTabWidget();
	tab->addTab(createPageStartup(), tr("Startup"));
	tab->addTab(createPageFolder(), tr("Folder"));
	tab->addTab(createPageView(), tr("View"));

	layout->addWidget(tab);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeOptionSetting::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &TeOptionSetting::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeOptionSetting::~TeOptionSetting()
{
}

void TeOptionSetting::updateSettings()
{
	QSettings settings;
	for (auto item = m_option.begin(); item != m_option.end(); ++item) {
		settings.setValue(item.key(),item.value());
	}
}

void TeOptionSetting::storeDefaultSettings()
{
	QSettings settings;

#define SETTING( str, default_value)  settings.setValue( str , settings.value( str , default_value ))

	SETTING(SETTING_STARTUP_MultiInstance, true);
	SETTING(SETTING_STARTUP_InitialFolderMode, TeSettings::INIT_FOLDER_MODE_SELECTED);
	SETTING(SETTING_STARTUP_InitialFolder, QDir::rootPath() );

#undef SETTING
}

void TeOptionSetting::accept()
{
	updateSettings();
	QDialog::accept();
}

QWidget * TeOptionSetting::createPageStartup()
{
	QSettings settings;
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();

	QCheckBox* cbox = new QCheckBox(tr("Allow multi instance."));
	cbox->setChecked(settings.value(SETTING_STARTUP_MultiInstance).toBool());
	connect(cbox, &QCheckBox::stateChanged, [this](int state) { m_option[SETTING_STARTUP_MultiInstance] =  (state == Qt::Checked); });
	layout->addWidget(cbox);

	QGroupBox* groupBox = new QGroupBox(tr("Startup Folder"));
	QVBoxLayout* boxLayout = new QVBoxLayout();

	QRadioButton* radio = new QRadioButton(tr("Hold previous state."));
	radio->setChecked(settings.value(SETTING_STARTUP_InitialFolderMode).toInt() == TeSettings::INIT_FOLDER_MODE_PREVIOUS);
	connect(radio, &QRadioButton::clicked, [this](bool checked) { if (checked) m_option[SETTING_STARTUP_InitialFolderMode] = TeSettings::INIT_FOLDER_MODE_PREVIOUS; });
	boxLayout->addWidget(radio);
	radio = new QRadioButton(tr("Selected Folder."));
	radio->setChecked(settings.value(SETTING_STARTUP_InitialFolderMode).toInt() == TeSettings::INIT_FOLDER_MODE_SELECTED);
	connect(radio, &QRadioButton::clicked, [this](bool checked) { if (checked) m_option[SETTING_STARTUP_InitialFolderMode] = TeSettings::INIT_FOLDER_MODE_SELECTED; });
	boxLayout->addWidget(radio);

	QHBoxLayout* editLayout = new QHBoxLayout();
	QLineEdit* edit = new QLineEdit(settings.value(SETTING_STARTUP_InitialFolder).toString());
	connect(edit, &QLineEdit::textChanged, [this](const QString& text) {m_option[SETTING_STARTUP_InitialFolder] = text; });
	editLayout->addWidget(edit);
	QPushButton* button = new QPushButton(tr("Find"));
	connect(button, &QPushButton::clicked, [edit,this](bool checked) 
	{
		TeSelectPathDialog dlg(this); 
		dlg.setTargetPath(edit->text());
		if (dlg.exec() == QDialog::Accepted) {
			edit->setText(dlg.targetPath());
		}
	});
	editLayout->addWidget(button);
	boxLayout->addLayout(editLayout);

	groupBox->setLayout(boxLayout);
	layout->addWidget(groupBox);

	page->setLayout(layout);
	return page;
}

QWidget * TeOptionSetting::createPageFolder()
{
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();



	page->setLayout(layout);
	return page;
}

QWidget * TeOptionSetting::createPageView()
{
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();



	page->setLayout(layout);
	return page;
}
