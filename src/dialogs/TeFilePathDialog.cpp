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

#include "TeFilePathDialog.h"
#include "TeSelectPathDialog.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QDir>
#include <QCompleter>
#include <QFileSystemModel>

TeFilePathDialog::TeFilePathDialog(QWidget *parent, const QStringList& extraFlags)
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

	//ExtraFlags
	QVBoxLayout* extra = new QVBoxLayout();
	for (int i = 0; i < extraFlags.count(); i++) {
		QCheckBox* check = new QCheckBox(extraFlags.at(i));
		m_extraFlags.append(check);
		extra->addWidget(check);
	}

	layout->addLayout(extra);

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

void TeFilePathDialog::setExtraFlag(int index, bool flag)
{
	if (index < m_extraFlags.count()) {
		m_extraFlags[index]->setChecked(flag);
	}
}

bool TeFilePathDialog::getExtraFlag(int index)
{
	if (index < m_extraFlags.count()) {
		return m_extraFlags[index]->isChecked();
	}
	return false;
}

void TeFilePathDialog::setFavorites(const QStringList& paths)
{
	m_favorites = paths;
}

void TeFilePathDialog::setHistory(const QStringList& paths)
{
	m_history = paths;
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
			else if (keyEvent->key() == Qt::Key_Up) {
				//Support show history when Up are pushed.
				mp_combo->clear();
				mp_combo->addItems(m_history);
				mp_combo->showPopup();
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Down) {
				//Support show favorite when Down are pushed.
				mp_combo->clear();
				mp_combo->addItems(m_favorites);
				mp_combo->showPopup();
				return true;
			}
		}
	}
	return false;
}

