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
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
	static bool m_tmpSelect = false;
	bool tmpSelect = m_tmpSelect;
	m_tmpSelect = false;

	if (selectionMode() == QListView::ExtendedSelection) {
		Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		if (event == nullptr) {
			//Suspend clear & select when setCurrentIndex()
			return QItemSelectionModel::NoUpdate;
		}else{
			switch (event->type()) {
			case QEvent::MouseButtonRelease: {
				Qt::MouseButton mousebutton = static_cast<const QMouseEvent*>(event)->button();
				const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
				const bool controlKeyPressed = modifiers & Qt::ControlModifier;
				if (!shiftKeyPressed && !controlKeyPressed && (mousebutton != Qt::MidButton)) {
					return QItemSelectionModel::NoUpdate;
				}
				break;
			}
			case QEvent::MouseButtonPress: {
				Qt::MouseButton mousebutton = static_cast<const QMouseEvent*>(event)->button();
				const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
				const bool controlKeyPressed = modifiers & Qt::ControlModifier;
				if (!shiftKeyPressed && !controlKeyPressed && (mousebutton != Qt::MidButton)) {
					if (selectionModel()->isSelected(index)) {
						return QItemSelectionModel::NoUpdate;
					}
					else {
						return QItemSelectionModel::Clear;
					}
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
