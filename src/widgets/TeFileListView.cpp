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

#include <QKeyEvent>
#include <QRubberBand>
#include <QApplication>
#include <qdebug.h>

/*!
	\class TeFileListView
	\breif 
 */

TeFileListView::TeFileListView(QWidget *parent)
	: QListView(parent)
{
	mp_folderView = nullptr;
	mp_rubberBand = new QRubberBand(QRubberBand::Rectangle,this);
	m_clearAndSelect_by_press = false;
	m_selectionMode = TeTypes::SELECTION_NONE;
	QListView::setSelectionMode(QAbstractItemView::NoSelection);
}

TeFileListView::~TeFileListView()
{
	delete mp_rubberBand;
}

TeFolderView * TeFileListView::folderView()
{
	return mp_folderView;
}

void TeFileListView::setFolderView(TeFolderView * view)
{
	mp_folderView = view;
}

void TeFileListView::setSelectionMode(TeTypes::SelectionMode mode)
{
	m_selectionMode = mode;
	m_pressedIndex = QModelIndex();
	switch (mode) {
	case TeTypes::SELECTION_NONE:
	case TeTypes::SELECTION_TABLE_ENGINE:
		QListView::setSelectionMode(QAbstractItemView::NoSelection);
		break;
	case TeTypes::SELECTION_EXPLORER:
		QListView::setSelectionMode(QAbstractItemView::ExtendedSelection);
		break;
	}
}

TeTypes::SelectionMode TeFileListView::selectionMode() const
{
	return m_selectionMode;
}

QRect TeFileListView::visualRect(const QModelIndex& index) const
{
	QRect rect = QListView::visualRect(index);
	rect.adjust(spacing(), 0, -spacing(), 0);
	return rect;
}

/*!
	add keyaction.
	If space key is pressed then current index is selected and move cursor to next entry.
	i.e. this function provide only move to next entry acton (this action not change selection). 
	     selection feature is implemented by selectionCommand().
 */
void TeFileListView::keyPressEvent(QKeyEvent *event)
{
	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		if (event) {
			QString str = model()->data(currentIndex(), Qt::DisplayRole).toString();
			if (str != ".." && str != ".") {
				switch (event->key()) {
				case Qt::Key_Up:
				case Qt::Key_Down:
				case Qt::Key_Left:
				case Qt::Key_Right:
					if (event->modifiers() == Qt::ShiftModifier) {
						if (m_pressedIndex != currentIndex()) {
							selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::Toggle);
						}
					}
					//Ctrl , No modifier are no effect.
					break;
				case Qt::Key_Space:
					if (m_pressedIndex != currentIndex()) {
						selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::Toggle);
					}
					break;
				}
			}
			switch (event->key()) {
			case Qt::Key_Shift:
			case Qt::Key_Control:
			case Qt::Key_Alt:
				break;
			default:
				m_pressedIndex = QModelIndex();
			}
		}

		QListView::keyPressEvent(event);

		QPersistentModelIndex newCurrent, current;
		if (event) {
			switch (event->key()) {
			case Qt::Key_Space:
				//select by space key.
				current = currentIndex();
				if (event->modifiers() == Qt::ShiftModifier) {
					newCurrent = model()->index(current.row() - 1, current.column(), rootIndex());
				}
				else {
					newCurrent = model()->index(current.row() + 1, current.column(), rootIndex());
				}
				if (newCurrent.isValid()) {
					selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
				}
				break;
			default:
				break;
			}
		}
	}
	else {
		QListView::keyPressEvent(event);
	}
}

void TeFileListView::mousePressEvent(QMouseEvent* event)
{
	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		QPoint pos = event->position().toPoint();
		QModelIndex index = indexAt(pos);
		if (selectionModel()->isSelected(index)) {
			m_pressedIndex = QModelIndex();
		}else{
			m_pressedIndex = index;
			if (isSelectionRectVisible()) {
				m_pressedPos = pos + QPoint(horizontalOffset(), verticalOffset());
				mp_rubberBand->setGeometry(QRect::span(pos, m_pressedPos).normalized());
				mp_rubberBand->show();
			}
		}

		Qt::KeyboardModifiers modifiers = event && event->isInputEvent()
			? static_cast<const QInputEvent*>(event)->modifiers()
			: QGuiApplication::keyboardModifiers();
		if (!index.isValid() && !(modifiers & (Qt::ShiftModifier | Qt::ControlModifier))) {
			clearSelection();
		}
	}
	QListView::mousePressEvent(event);

	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		QModelIndex topIndex = model()->index(0, 0, rootIndex());
		if (".." == model()->data(topIndex, Qt::DisplayRole).toString()) {
			selectionModel()->select(topIndex, QItemSelectionModel::Deselect);
		}
	}
}

void TeFileListView::mouseMoveEvent(QMouseEvent* event)
{
	if (mp_rubberBand->isVisible()) {
		repaint(mp_rubberBand->normalGeometry());
		QPoint pos = event->position().toPoint();
		QPoint startPos = m_pressedPos - QPoint(horizontalOffset(), verticalOffset());
		mp_rubberBand->setGeometry(QRect::span(startPos,pos).normalized());
	}
	QListView::mouseMoveEvent(event);
}

void TeFileListView::mouseReleaseEvent(QMouseEvent* event)
{
	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		mp_rubberBand->hide();
	}
	QListView::mouseReleaseEvent(event);

	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		QModelIndex topIndex = model()->index(0, 0, rootIndex());
		if (".." == model()->data(topIndex, Qt::DisplayRole).toString()) {
			selectionModel()->select(topIndex, QItemSelectionModel::Deselect);
		}

		if (selectionModel()->isSelected(currentIndex())) {
			selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::Select);
		}
		else {
			selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::NoUpdate);
		}
	}
}

/*!
	config selection patten for events.
 */
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {

		Qt::KeyboardModifiers modifiers = event && event->isInputEvent()
            ? static_cast<const QInputEvent*>(event)->modifiers()
            : QGuiApplication::keyboardModifiers();

		QString str = model()->data(index, Qt::DisplayRole).toString();
		bool indexInvalid = !index.isValid() || (str == "..");
		bool indexSelected = selectionModel()->isSelected(index);
        if (event) {
            switch (event->type()) {
            case QEvent::MouseMove: {
				if (modifiers & Qt::ShiftModifier) {
					return QItemSelectionModel::SelectCurrent;
				}
				else {
					return QItemSelectionModel::ToggleCurrent;
				}

                break;
            }
            case QEvent::MouseButtonPress: {
				if (modifiers & Qt::ShiftModifier) {
					if (index.isValid()) {
						return QItemSelectionModel::SelectCurrent;
					}
					else {
						return QItemSelectionModel::Select;
					}

				}
				else if (modifiers & Qt::ControlModifier) {
					return QItemSelectionModel::Toggle;
				}
				else {
					if (indexInvalid) {
						return QItemSelectionModel::Clear;
					}
					else if (indexSelected) {
						return QItemSelectionModel::NoUpdate;
					}
					else {
						return QItemSelectionModel::ClearAndSelect;
					}
				}
                break;
            }
            case QEvent::MouseButtonRelease: {
				break;
			}
            case QEvent::KeyPress:
            default:
                break;
            }
        }

		return QItemSelectionModel::NoUpdate;
	}
	return QListView::selectionCommand(index, event);
}
