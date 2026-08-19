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
#include "TeSettings.h"

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
	m_anchorIndex = QModelIndex();
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

void TeFileListView::setModel(QAbstractItemModel* model)
{
	QListView::setModel(model);
	m_anchorIndex = QModelIndex();
}

void TeFileListView::setRootIndex(const QModelIndex& index)
{
	QListView::setRootIndex(index);
	m_anchorIndex = QModelIndex();
}

/*!
	Sole current-navigation anchor-update path: keyboard moves, mouse clicks/
	drags (via QListView::mousePressEvent()/mouseMoveEvent()), and programmatic
	setCurrentIndex() calls all funnel through here. Synchronization is skipped
	while a CurrentChangedBlockGuard is active, so mousePressEvent()'s explicit
	applyPressSelection() logic keeps sole ownership of m_anchorIndex for
	Ctrl/Shift/right-click semantics.
 */
void TeFileListView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	QListView::currentChanged(current, previous);

	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE) && !m_currentChangedBlocked) {
		m_anchorIndex = isSelectableIndex(current) ? QPersistentModelIndex(current) : QPersistentModelIndex();
	}
}

bool TeFileListView::isSelectableIndex(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return false;
	}
	return model()->data(index, Qt::DisplayRole).toString() != QStringLiteral("..");
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

		// currentChanged() is the sole current-navigation anchor-update path: any
		// resulting setCurrentIndex() (from QListView::keyPressEvent() above or
		// from the Space-key handling below) synchronizes m_anchorIndex itself.
		if (event) {
			QPersistentModelIndex newCurrent, current;
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
		Qt::KeyboardModifiers modifiers = event->modifiers();
		QPoint pos = event->position().toPoint();
		QModelIndex index = indexAt(pos);
		bool wasSelected = index.isValid() && selectionModel()->isSelected(index);

		// Left and right press both establish/clear the "temporary single
		// selection" tracked by m_pressedIndex, so a following arrow-key move
		// clears it regardless of which button created it; only left press
		// starts the drag rubber-band.
		if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
			if (wasSelected) {
				m_pressedIndex = QModelIndex();
			}
			else {
				m_pressedIndex = index;
				if (event->button() == Qt::LeftButton && isSelectionRectVisible()) {
					m_pressedPos = pos + QPoint(horizontalOffset(), verticalOffset());
					mp_rubberBand->setGeometry(QRect::span(pos, m_pressedPos).normalized());
					mp_rubberBand->show();
				}
			}
		}

		// selectionCommand() returns NoUpdate for MouseButtonPress in this mode,
		// so the base call only retains Qt's current/pressed/edit/drag machinery;
		// selection itself is applied explicitly below, independent of Qt's
		// platform-dependent internal anchor handling. currentChanged()'s anchor
		// sync is suspended for the duration of the base call so applyPressSelection()
		// below sees the pre-click anchor, not one clobbered by the base class's
		// own setCurrentIndex().
		{
			CurrentChangedBlockGuard guard(m_currentChangedBlocked);
			QListView::mousePressEvent(event);
		}

		applyPressSelection(index, wasSelected, event->button(), modifiers);

		//deselect ".." entry.
		QModelIndex topIndex = model()->index(0, 0, rootIndex());
		if (".." == model()->data(topIndex, Qt::DisplayRole).toString()) {
			selectionModel()->select(topIndex, QItemSelectionModel::Deselect);
		}
		return;
	}
	QListView::mousePressEvent(event);
}

/*!
	Applies the TABLE_ENGINE click selection rule explicitly (see
	doc/markdown/widgets/TeFileListView.md), independent of Qt's
	platform-dependent internal anchor handling.
 */
void TeFileListView::applyPressSelection(const QModelIndex& index, bool wasSelected, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
	bool validTarget = isSelectableIndex(index);

	if (button == Qt::LeftButton) {
		if (modifiers & Qt::ShiftModifier) {
			if (!validTarget) {
				// blank / ".." target: keep the current selection unchanged.
				return;
			}
			// With no existing selection there is nothing to extend from, so
			// the click establishes a fresh anchor at the target itself
			// (matches a plain click) rather than reusing a stale anchor.
			// The parent check guards against a stale anchor left over from a
			// different folder/root that setRootIndex()/setModel() failed to
			// invalidate (defensive; those overrides already reset it).
			QModelIndex anchor;
			if (m_anchorIndex.isValid() && QModelIndex(m_anchorIndex).parent() == rootIndex() && selectionModel()->hasSelection()) {
				anchor = QModelIndex(m_anchorIndex);
			}
			else {
				anchor = index;
				m_anchorIndex = index;
			}
			int r1 = qMin(anchor.row(), index.row());
			int r2 = qMax(anchor.row(), index.row());
			QModelIndex parent = rootIndex();
			QItemSelection range(model()->index(r1, index.column(), parent), model()->index(r2, index.column(), parent));
			selectionModel()->select(range, (modifiers & Qt::ControlModifier) ? QItemSelectionModel::Select : QItemSelectionModel::ClearAndSelect);
			// anchor is preserved for Shift (and Ctrl+Shift) so repeated
			// Shift-clicks keep extending the range from the same origin.
			return;
		}

		if (modifiers & Qt::ControlModifier) {
			if (validTarget) {
				selectionModel()->select(index, QItemSelectionModel::Toggle);
			}
			// anchor is preserved for Ctrl.
			return;
		}

		// Unmodified click.
		if (!validTarget) {
			clearSelection();
			m_anchorIndex = QModelIndex();
			return;
		}
		if (!wasSelected) {
			selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
		}
		// else: clicking an already-selected item keeps the selection as-is.
		m_anchorIndex = index;
		return;
	}

	if (button == Qt::RightButton) {
		if (!validTarget) {
			return;
		}
		if (!wasSelected) {
			selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
		}
		// else: right-clicking an already-selected item preserves the selection.
		// The anchor is intentionally left untouched by right-click.
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
	// Suspend anchor sync: base-class hover-driven current changes during a
	// rubber-band drag must not move the anchor established at press time.
	CurrentChangedBlockGuard guard(m_currentChangedBlocked);
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
	if ((QListView::selectionMode() == QListView::NoSelection) && (selectionMode() == TeTypes::SELECTION_TABLE_ENGINE)) {
		// MouseMove (rubber-band drag hover) is the only case where the base
		// class still drives selection in this mode; every other case
		// (MouseButtonPress/Release/DblClick, KeyPress, and event == nullptr for
		// programmatic current-index changes) is applied explicitly by
		// mousePressEvent()/keyPressEvent(), so the base class must not touch
		// selection here. Modifiers are read only from the event itself; no
		// QGuiApplication fallback is used, keeping the behavior deterministic
		// and platform independent.
		if (event && event->type() == QEvent::MouseMove) {
			Qt::KeyboardModifiers modifiers = static_cast<const QInputEvent*>(event)->modifiers();
			return (modifiers & Qt::ShiftModifier) ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::ToggleCurrent;
		}

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

void TeFileListView::setFileNameWidthLimits(int minChars, int maxChars)
{
	m_minFileNameWidthChars = qBound(TeSettings::FILENAME_WIDTH_DISABLED, minChars,
		TeSettings::MAX_FILENAME_WIDTH_CHARS);
	m_maxFileNameWidthChars = qBound(TeSettings::FILENAME_WIDTH_DISABLED, maxChars,
		TeSettings::MAX_FILENAME_WIDTH_CHARS);
	if (m_maxFileNameWidthChars > TeSettings::FILENAME_WIDTH_DISABLED
		&& m_minFileNameWidthChars > m_maxFileNameWidthChars) {
		m_minFileNameWidthChars = m_maxFileNameWidthChars;
	}

	TeFileItemDelegate* delegate = qobject_cast<TeFileItemDelegate*>(itemDelegate());
	if (delegate) {
		delegate->setFileNameWidthLimits(m_minFileNameWidthChars, m_maxFileNameWidthChars);
	}
	doItemsLayout();
	updateGeometry();
	viewport()->update();
}

int TeFileListView::minFileNameWidthChars() const
{
	return m_minFileNameWidthChars;
}

int TeFileListView::maxFileNameWidthChars() const
{
	return m_maxFileNameWidthChars;
}
