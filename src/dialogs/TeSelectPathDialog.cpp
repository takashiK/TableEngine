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
#include "TeSelectPathDialog.h"
#include "widgets/TeFileTreeView.h"

#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QDir>
#include <QCompleter>

TeSelectPathDialog::TeSelectPathDialog(QWidget *parent)
	: QDialog(parent)
{
	//Except Help Button
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QFileSystemModel* model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	model->setRootPath("");

	//EditBox for file path
	QVBoxLayout *layout = new QVBoxLayout();
	mp_edit = new QLineEdit();
	//link path string to current entry of treeview.
	connect(mp_edit, &QLineEdit::textChanged, [this](const QString& text) {setTargetPath(QDir::fromNativeSeparators(text)); });
	layout->addWidget(mp_edit);

	//Support path completion.
	QCompleter* completer = new QCompleter(model);
	completer->setCompletionMode(QCompleter::InlineCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	mp_edit->setCompleter(completer);


	//Folder tree
	model = new QFileSystemModel();
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	mp_tree = new TeFileTreeView();
	mp_tree->setModel(model);
	mp_tree->setVisualRootIndex(model->setRootPath(u8""));
	for (int i = 1; i<mp_tree->header()->count(); i++) {
		mp_tree->header()->setSectionHidden(i, true);
	}
	mp_tree->setHeaderHidden(true);
	mp_tree->installEventFilter(this);
	mp_tree->setAutoScroll(true);

	connect(mp_tree->selectionModel(), &QItemSelectionModel::currentChanged,
		[this,model](const QModelIndex &current, const QModelIndex &/*previous*/)
	    { setTargetPath(model->filePath(current)); });

	layout->addWidget(mp_tree);

	//Regist OK and Cancel.
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
	setTabOrder(mp_tree, buttonBox->button(QDialogButtonBox::Ok));
	setTabOrder(buttonBox->button(QDialogButtonBox::Ok), buttonBox->button(QDialogButtonBox::Cancel));
	setTabOrder(buttonBox->button(QDialogButtonBox::Cancel), mp_edit);
}

TeSelectPathDialog::~TeSelectPathDialog()
{
}

void TeSelectPathDialog::setTargetPath(const QString & path)
{
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(mp_tree->model());
	QModelIndex index = model->index(path);
	if (index.isValid()) {
		mp_tree->setCurrentIndex(index);
		mp_tree->scrollTo(mp_tree->currentIndex(),QAbstractItemView::PositionAtCenter);
	}
	mp_edit->setText(QDir::toNativeSeparators(path));
}

QString TeSelectPathDialog::targetPath()
{
	return QDir::cleanPath(QDir::fromNativeSeparators(mp_edit->text()));
}
