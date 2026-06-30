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
#include "TeFileItemDelegate.h"

#include <QKeyEvent>
#include <QRubberBand>
#include <QApplication>
#include <qdebug.h>
/**
 * @file TeFileListView.cpp
 * @brief Implementation of TeFileListView.
 * @ingroup widgets
 */

/*!
	\class TeFileListView
	\breif 
 */

TeFileListView::TeFileListView(QWidget *parent)
	: QListView(parent)
{
	mp_rubberBand = new QRubberBand(QRubberBand::Rectangle,this);
	QListView::setSelectionMode(QAbstractItemView::NoSelection);
	setItemDelegate(new TeFileItemDelegate(this));

//	QListView::setStyleSheet("QListView::item::focus { background: palette(highlight); }");
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
					else {
						if (m_pressedIndex == currentIndex() && selectionModel()->isSelected(currentIndex()) && selectionModel()->selectedIndexes().size() == 1) {
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
				default:
					// Incremental inline search (QAbstractItemView::keyboardSearch)
					// is triggered by a printable character without Ctrl/Alt/Meta.
					// It moves the cursor just like an arrow key, so the temporary
					// single selection made by a preceding click must be cancelled
					// (consistent with the arrow-key move behavior).
					if (!event->text().isEmpty()
						&& event->text().at(0).isPrint()
						&& !(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
						if (m_pressedIndex == currentIndex() && selectionModel()->isSelected(currentIndex()) && selectionModel()->selectedIndexes().size() == 1) {
							selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::Toggle);
						}
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
			case Qt::Key_Up:
			case Qt::Key_Down:
			case Qt::Key_Left:
			case Qt::Key_Right:
				// change current index by arrow keys.
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
	Qt::KeyboardModifiers modifiers = event && event->isInputEvent()
		? static_cast<const QInputEvent*>(event)->modifiers()
		: QGuiApplication::keyboardModifiers();

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

		if (!index.isValid() && !(modifiers & (Qt::ShiftModifier | Qt::ControlModifier))) {
			clearSelection();
		}
	}
	QListView::mousePressEvent(event);

	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		//deselect ".." entry.
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
	}
}

/*!
	config selection patten for events.
 */
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
	//qDebug() << "event type:" << (event ? event->type() : QEvent::None) << "index:" << index << "selected:" << selectionModel()->isSelected(index);
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
					//qDebug() << "MouseMove ShiftModifier SelectCurrent";
					return QItemSelectionModel::SelectCurrent;
				}
				else {
					//qDebug() << "MouseMove No Modifier ToggleCurrent";
					return QItemSelectionModel::ToggleCurrent;
				}

                break;
            }
            case QEvent::MouseButtonPress: {
				if (modifiers & Qt::ShiftModifier) {
					if (index.isValid()) {
						//qDebug() << "MouseButtonPress ShiftModifier indexValid SelectCurrent";
						return QItemSelectionModel::SelectCurrent;
					}
					else {
						//qDebug() << "MouseButtonPress ShiftModifier indexInvalid Select";
						return QItemSelectionModel::Select;
					}

				}
				else if (modifiers & Qt::ControlModifier) {
					//qDebug() << "MouseButtonPress ControlModifier Toggle";
					return QItemSelectionModel::Toggle;
				}
				else {
					if (indexInvalid) {
						//qDebug() << "MouseButtonPress No Modifier indexInvalid Clear";
						return QItemSelectionModel::Clear;
					}
					else if (indexSelected) {
						//qDebug() << "MouseButtonPress No Modifier indexSelected NoUpdate";
						return QItemSelectionModel::NoUpdate;
					}
					else {
						//qDebug() << "MouseButtonPress No Modifier ClearAndSelect";
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

			return QItemSelectionModel::NoUpdate;
        }

		// event == nullptr means the selection flags are requested for a
		// programmatic current-index change made through
		// QAbstractItemView::setCurrentIndex(). This happens in two distinct
		// situations that must behave differently:
		//
		//  1. Mouse Shift+click range selection. While the left button is held
		//     the view extends the selection from the anchor to the clicked
		//     item, so SelectCurrent must be returned (legacy behavior).
		//  2. Incremental inline search (QAbstractItemView::keyboardSearch),
		//     triggered by typing characters. The Shift key is often physically
		//     held while typing, but the cursor jump must NOT alter the
		//     selection.
		//
		// No mouse button is pressed during keyboard search, so the live mouse
		// button state distinguishes the two cases reliably.
		if ((modifiers & Qt::ShiftModifier) && (QGuiApplication::mouseButtons() != Qt::NoButton)) {
			//qDebug() << "Programmatic mouse Shift change SelectCurrent";
			return QItemSelectionModel::SelectCurrent;
		}

		//qDebug() << "Programmatic current change NoUpdate";
		return QItemSelectionModel::NoUpdate;
	}

	auto flags = QListView::selectionCommand(index, event);

	//qDebug() << "QListView::selectionCommand flags:" << flags;
	return flags;
}

QModelIndex TeFileListView::indexAt(const QPoint &point) const
{
	QModelIndex index = QListView::indexAt(point);
	QRect rect = visualRect(index);
	// unsense margin for empty space between items.
	// this is convinient for user to select region between items.
	rect.adjust(2, 2, -2, -2);
	if (!rect.contains(point)) {
		return QModelIndex();
	}

	return index;
}

void TeFileListView::setFileViewMode(TeTypes::FileInfoFlags infoFlags, TeTypes::FileViewMode viewMode)
{
	//file info
	TeFileItemDelegate* delegate = qobject_cast<TeFileItemDelegate*>(itemDelegate());
	if (delegate) {
		delegate->setInfoFlags(infoFlags);
	}

	//view mode
	switch (viewMode) {
	case TeTypes::FILEVIEW_SMALL_ICON:
		QListView::setViewMode(QListView::ListMode);
		QListView::setFlow(QListView::TopToBottom);
		QListView::setWrapping(true);
		QListView::setIconSize(QSize(-1, -1));
		QListView::setWordWrap(false);
		QListView::setResizeMode(QListView::Adjust);
		break;
	case TeTypes::FILEVIEW_LARGE_ICON:
		QListView::setViewMode(QListView::IconMode);
		QListView::setFlow(QListView::LeftToRight);
		QListView::setWrapping(true);
		QListView::setIconSize(QSize(64, 64));
		QListView::setWordWrap(true);
		QListView::setResizeMode(QListView::Adjust);
		QListView::setUniformItemSizes(true);
		break;
	case TeTypes::FILEVIEW_HUGE_ICON:
		QListView::setViewMode(QListView::IconMode);
		QListView::setFlow(QListView::LeftToRight);
		QListView::setWrapping(true);
		QListView::setIconSize(QSize(192, 192));
		QListView::setWordWrap(true);
		QListView::setResizeMode(QListView::Adjust);
		QListView::setUniformItemSizes(true);
		break;
	case TeTypes::FILEVIEW_DETAIL_LIST:
		QListView::setViewMode(QListView::ListMode);
		QListView::setFlow(QListView::TopToBottom);
		QListView::setWrapping(false);
		QListView::setIconSize(QSize(-1, -1));
		QListView::setWordWrap(false);
		QListView::setResizeMode(QListView::Adjust);
		break;
	}
}
