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
#include "TeFileListView.h"

#include <QApplication>
#include <QKeyEvent>

/*!
	\class TeFileListView
	\breif 
 */

TeFileListView::TeFileListView(QWidget *parent)
	: QListView(parent)
{
	mp_folderView = nullptr;
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
	QListView::keyPressEvent(event);

	QPersistentModelIndex newCurrent, current;
	if (event) {
		switch (event->type()) {
		case QEvent::KeyPress: {
			switch (static_cast<const QKeyEvent*>(event)->key()) {
			case Qt::Key_Space:
				//select by space key.
				current = currentIndex();
				newCurrent = model()->index(current.row() + 1, current.column(), rootIndex());
				if (newCurrent.isValid()) {
					selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

/*!
	config selection patten for events.
 */
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
	if (selectionMode() == QListView::ExtendedSelection) {
		Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		if (event == nullptr) {
			//Suspend clear & select when setCurrentIndex()
			return QItemSelectionModel::NoUpdate;
		}else{
			switch (event->type()) {
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonPress: {
				modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
				const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
				const bool controlKeyPressed = modifiers & Qt::ControlModifier;
				if (!shiftKeyPressed && !controlKeyPressed) {
					//except single click selection.
					return QItemSelectionModel::NoUpdate;
				}
				break;
			}
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
