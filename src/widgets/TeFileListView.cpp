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

#include "TeFileListView.h"

#include <QApplication>
#include <QKeyEvent>
#include <qdebug.h>

/*!
	\class TeFileListView
	\breif 
 */

TeFileListView::TeFileListView(QWidget *parent)
	: QListView(parent)
{
	mp_folderView = nullptr;
	m_clearAndSelect_by_press = false;
}

TeFileListView::~TeFileListView()
{
}

TeFolderView * TeFileListView::folderView()
{
	return mp_folderView;
}

void TeFileListView::setFolderView(TeFolderView * view)
{
	mp_folderView = view;
}

/*!
	add keyaction.
	If space key is pressed then current index is selected and move cursor to next entry.
	i.e. this function provide only move to next entry acton (this action not change selection). 
	     selection feature is implemented by selectionCommand().
 */
void TeFileListView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Control:
	case Qt::Key_Shift:
	case Qt::Key_Alt:
		break;
	default:
		selectionModel()->select(m_preesedIndex, QItemSelectionModel::Deselect);
		break;
	}

	QListView::keyPressEvent(event);

	switch (event->key()) {
	case Qt::Key_Control:
	case Qt::Key_Shift:
	case Qt::Key_Alt:
		break;
	default:
		m_preesedIndex = QModelIndex();
		break;
	}

	QPersistentModelIndex newCurrent, current;
	if (event) {
		switch (event->key()) {
		case Qt::Key_Space:
			//select by space key.
			current = currentIndex();
			newCurrent = model()->index(current.row() + 1, current.column(), rootIndex());
			if (newCurrent.isValid()) {
				selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
			}
			break;
		default:
			break;
		}
	}
}

void TeFileListView::mousePressEvent(QMouseEvent* event)
{
	QPoint pos = event->pos();
	QModelIndex index = indexAt(pos);
	QItemSelectionModel::SelectionFlags flags = selectionCommand(index, event);
	if (flags.testFlag(QItemSelectionModel::ClearAndSelect)) {
		m_preesedIndex = index;
	}
	else if(index == m_preesedIndex && flags.testFlag(QItemSelectionModel::NoUpdate)){
		//no more action
	}
	else {
		m_preesedIndex = QModelIndex();
	}
	QListView::mousePressEvent(event);
}

/*!
	config selection patten for events.
 */
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
	if (selectionMode() == QListView::ExtendedSelection) {
		Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		if (event == nullptr) {
			//Suspend clear & select when setCurrentIndex()
			return QItemSelectionModel::NoUpdate;
		}else{
			switch (event->type()) {
			case QEvent::KeyPress: {
				switch (static_cast<const QKeyEvent*>(event)->key()) {
				case Qt::Key_Down:
				case Qt::Key_Up:
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_PageUp:
				case Qt::Key_PageDown:
					//except release selection when cursor is moved.
					return QItemSelectionModel::NoUpdate;
				case Qt::Key_Space:
					//change selection by space key.
					return QItemSelectionModel::Toggle;
				}
			}
			default:
				//Action of other cases are same as Qt Default.
				return QListView::selectionCommand(index, event);
			}
		}
	}
	return QListView::selectionCommand(index, event);
}
