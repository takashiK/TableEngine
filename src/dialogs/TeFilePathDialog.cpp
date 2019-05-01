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
#include "TeFilePathDialog.h"
#include "TeSelectPathDialog.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QDir>
#include <QCompleter>
#include <QFileSystemModel>

TeFilePathDialog::TeFilePathDialog(QWidget *parent)
	: QDialog(parent)
{
	//Except Help button
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout *layout = new QVBoxLayout();

	//CommboBox for File path.
	mp_combo = new QComboBox();
	mp_combo->setEditable(true);
	mp_combo->setMinimumWidth(300);
	mp_combo->installEventFilter(this);

	QFileSystemModel* model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	model->setRootPath("");
	QCompleter* completer = new QCompleter(model);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	mp_combo->setCompleter(completer);

	layout->addWidget(mp_combo);

	QLabel* label = new QLabel(tr("Shift+Enter: Select path  Up: History  Down: Favorite"));
	layout->addWidget(label);
	layout->setAlignment(label, Qt::AlignHCenter);

	//Register OK and Cancel.
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);


	setLayout(layout);
}

TeFilePathDialog::~TeFilePathDialog()
{
}

void TeFilePathDialog::setCurrentPath(const QString & path)
{
	m_currentPath = path;
}

QString TeFilePathDialog::currentPath()
{
	return m_currentPath;
}

void TeFilePathDialog::setTargetPath(const QString & path)
{
	mp_combo->setCurrentText(QDir::toNativeSeparators(path));
}

QString TeFilePathDialog::targetPath()
{
	if (m_currentPath.isEmpty()) {
		return QDir::cleanPath(QDir::fromNativeSeparators(mp_combo->currentText()));
	}
	else {
		QDir dir(m_currentPath);
		QString path = QDir::fromNativeSeparators(mp_combo->currentText());
		return QDir::cleanPath(dir.absoluteFilePath(path));
	}
}

bool TeFilePathDialog::eventFilter(QObject * obj, QEvent * event)
{
	if (obj == mp_combo) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			//Support show folder tree selection when Shift+Enter are pushed.
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_Return)) {
				TeSelectPathDialog dlg(this);
				dlg.setWindowTitle(tr("Select Path"));
				dlg.setTargetPath(targetPath());
				if (dlg.exec() == QDialog::Accepted) {
					setTargetPath(dlg.targetPath());
				}
				return true;
			}
		}
	}
	return false;
}

