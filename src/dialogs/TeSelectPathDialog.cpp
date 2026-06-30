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

#include "TeSelectPathDialog.h"
#include "widgets/TeFileTreeView.h"
#include "TeSettings.h"

#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QDir>
#include <QCompleter>
#include <QShowEvent>
#include <QScopedValueRollback>
#include <QTimer>

/**
 * @file TeSelectPathDialog.cpp
 * @brief Declaration of TeSelectPathDialog.
 * @ingroup dialogs
 */


TeSelectPathDialog::TeSelectPathDialog(QWidget *parent)
	: QDialog(parent)
{
	setMinimumWidth(TeSettings::dialogMinimumWidth());
	//Except Help Button
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

	QFileSystemModel* model = new QFileSystemModel(this);
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	model->setRootPath("");

	//EditBox for file path
	QVBoxLayout *layout = new QVBoxLayout();
	mp_edit = new QLineEdit();
	//link path string to current entry of treeview.
	connect(mp_edit, &QLineEdit::textChanged, this, [this](const QString& text) {
		if (m_syncing)
			return;
		QScopedValueRollback<bool> guard(m_syncing, true);
		editToTree(QDir::fromNativeSeparators(text));
	});
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
	mp_tree->setAutoScroll(true);

	//Enable horizontal scrolling for deep folder hierarchies.
	mp_tree->header()->setStretchLastSection(false);
	mp_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	mp_tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mp_tree->setTextElideMode(Qt::ElideNone);

	connect(mp_tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
		[this](const QModelIndex &current, const QModelIndex &/*previous*/)
	    {
			if (m_syncing)
				return;
			QScopedValueRollback<bool> guard(m_syncing, true);
			treeToEdit(current);
		});

	//Re-resolve and re-scroll while the filesystem model keeps loading lazily.
	connect(model, &QFileSystemModel::directoryLoaded, this,
		[this](const QString& /*path*/)
	    {
			tryReveal();
		});

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
	if (m_syncing)
		return;
	QScopedValueRollback<bool> guard(m_syncing, true);
	editToTree(QDir::fromNativeSeparators(path));
}

QString TeSelectPathDialog::targetPath()
{
	return QDir::cleanPath(QDir::fromNativeSeparators(mp_edit->text()));
}

void TeSelectPathDialog::editToTree(const QString & path)
{
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(mp_tree->model());
	if (model) {
		QModelIndex index = model->index(path);
		if (index.isValid())
			mp_tree->setCurrentIndex(index);
		//Remember the target and (re)try scrolling. QFileSystemModel loads lazily,
		//so the row may not be laid out yet; tryReveal()/directoryLoaded will retry.
		scheduleReveal(path);
	}
	mp_edit->setText(QDir::toNativeSeparators(path));
}

void TeSelectPathDialog::treeToEdit(const QModelIndex & index)
{
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(mp_tree->model());
	if (!model || !index.isValid())
		return;
	//User-driven selection: cancel any pending auto-scroll and just sync the edit box.
	m_revealTarget.clear();
	mp_edit->setText(QDir::toNativeSeparators(model->filePath(index)));
}

void TeSelectPathDialog::scheduleReveal(const QString & path)
{
	m_revealTarget = path;
	tryReveal();
}

void TeSelectPathDialog::tryReveal()
{
	if (m_revealTarget.isEmpty() || !isVisible())
		return;
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(mp_tree->model());
	if (!model)
		return;
	QModelIndex index = model->index(m_revealTarget);
	if (!index.isValid())
		return; //Nodes not created yet; retry on directoryLoaded.

	QScopedValueRollback<bool> guard(m_syncing, true);

	//Expand ancestors so the target has a visible row that scrollTo can locate.
	QList<QModelIndex> ancestors;
	for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent())
		ancestors.prepend(parent);
	for (const QModelIndex& parent : ancestors)
		mp_tree->expand(parent);

	mp_tree->setCurrentIndex(index);
	mp_tree->scrollTo(index, QAbstractItemView::PositionAtCenter);

	//If the row is laid out, scrolling has taken effect. Otherwise the child rows are
	//still loading asynchronously; keep the target and retry on the next directoryLoaded.
	if (!mp_tree->visualRect(index).isEmpty())
		m_revealTarget.clear();
}

void TeSelectPathDialog::showEvent(QShowEvent * event)
{
	QDialog::showEvent(event);
	//Scroll to and focus the current path once the dialog is actually visible.
	scheduleReveal(targetPath());
	mp_tree->setFocus();
	//Re-scroll after the layout/viewport is finalized (deferred to the next event loop cycle).
	QTimer::singleShot(0, this, [this]() { tryReveal(); });
}
