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

#pragma once

#include <QListView>
#include <QPersistentModelIndex>
#include "TeTypes.h"

/**
 * @file TeFileListView.h
 * @brief File list-view widget with TableEngine custom selection mode.
 * @ingroup widgets
 *
 * @details Declares TeFileListView, which extends QListView with an additional
 * "TableEngine" selection mode that is distinct from Qt's built-in
 * selection modes.  The detailed selection-mode specification is documented in
 * the design document referenced below.
 *
 * @see doc/markdown/widgets/TeFileListView.md
 */

class QRubberBand;
class TeFolderView;

/**
 * @class TeFileListView
 * @brief QListView subclass supporting the TableEngine selection mode.
 * @ingroup widgets
 *
 * @details TeFileListView provides three selection modes:
 * - @c NONE        — selection is disabled.
 * - @c EXPLORER    — follows Windows Explorer behavior.
 * - @c TABLE_ENGINE — custom mode that toggles item selection on click/Space
 *   without losing other selections unless the user explicitly clears them.
 *
 * The TABLE_ENGINE mode stores extra state (m_pressedIndex, m_pressedPos)
 * to distinguish a click from a drag-start, and uses mp_rubberBand for
 * rubber-band selection.
 *
 * indexAt() is overridden to extend the hit-test area inward by a small
 * margin so that clicks on the edge gutter return QModelIndex() ("no item").
 *
 * @see TeFolderView, TeFileSortProxyModel
 * @see doc/markdown/widgets/TeFileListView.md
 */
class TeFileListView : public QListView
{
	Q_OBJECT

public:
	TeFileListView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileListView();

	/** @brief Returns the owning TeFolderView. */
	virtual TeFolderView* folderView();

	/**
	 * @brief Sets the owning TeFolderView back-reference.
	 * @param view The parent TeFolderView.
	 */
	void setFolderView(TeFolderView* view);

	/** @brief Returns the active selection mode. */
	TeTypes::SelectionMode selectionMode() const;

	/**
	 * @brief Returns the item at @p point, shrinking the hit-test area by a
	 * small margin so that the gutter around each item cell returns an invalid
	 * index.
	 */
	virtual QModelIndex	indexAt(const QPoint &point) const;
	/** @brief Resets the range-selection anchor when the model changes. */
	virtual void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;

	/** @brief Resets the range-selection anchor when the root index changes. */
	virtual void setRootIndex(const QModelIndex& index) Q_DECL_OVERRIDE;
public slots:
	/**
	 * @brief Switches the active selection mode.
	 * @param mode The selection mode to use.
	 */
	void setSelectionMode(TeTypes::SelectionMode mode);

	/**
	 * @brief Configures the view display format.
	 * @param infoFlags Which file attributes to show per item.
	 * @param viewMode  Icon vs. Detail layout.
	 */
	void setFileViewMode(TeTypes::FileInfoFlags infoFlags, TeTypes::FileViewMode viewMode);

	/**
	 * @brief Applies filename display-width limits to non-icon list layouts.
	 * @param minChars Minimum width in active-font "M" units; zero disables it.
	 * @param maxChars Maximum width in active-font "M" units; zero disables it.
	 */
	void setFileNameWidthLimits(int minChars, int maxChars);
	/** @brief Returns the applied minimum filename width in "M" units. */
	int minFileNameWidthChars() const;
	/** @brief Returns the applied maximum filename width in "M" units. */
	int maxFileNameWidthChars() const;

protected:
	/** @brief Handles keyboard navigation; implements TABLE_ENGINE Space-key toggle. */
	virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	/** @brief Records the pressed item/position; starts rubber-band for TABLE_ENGINE mode. */
	virtual void mousePressEvent(QMouseEvent* event);
	/** @brief Updates rubber-band geometry and hover selection in TABLE_ENGINE mode. */
	virtual void mouseMoveEvent(QMouseEvent* event);
	/** @brief Finalises selection and hides the rubber-band on button release. */
	virtual void mouseReleaseEvent(QMouseEvent* event);
	/**
	 * @brief Returns selection flags appropriate for the active selection mode.
	 *
	 * In TABLE_ENGINE mode the base-class logic is bypassed to prevent
	 * unintended selection clearing.
	 */
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = Q_NULLPTR) const;
	/**
	 * @brief Synchronizes m_anchorIndex to the new current index in TABLE_ENGINE
	 * mode, unless suspended by an active CurrentChangedBlockGuard. This is the
	 * sole current-navigation anchor-update path (keyboard, mouse, and
	 * programmatic setCurrentIndex() all funnel through here).
	 */
	virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous) Q_DECL_OVERRIDE;

private:
	/** @brief Returns whether @p index is a valid, selectable item (excludes ".."). */
	bool isSelectableIndex(const QModelIndex& index) const;
	/**
	 * @brief Applies the TABLE_ENGINE click selection rule explicitly, decoupled
	 * from Qt's platform-dependent internal anchor handling.
	 * @param index     Target item under the cursor (may be invalid).
	 * @param wasSelected Whether @p index was already selected before this press.
	 * @param button    Mouse button that triggered the press.
	 * @param modifiers Keyboard modifiers held at press time.
	 */
	void applyPressSelection(const QModelIndex& index, bool wasSelected, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

	/**
	 * @brief RAII guard that suspends currentChanged()'s anchor synchronization
	 * for its lifetime, so explicit press/drag selection logic (which reads or
	 * sets m_anchorIndex itself) is not clobbered by the base class's own
	 * setCurrentIndex() calls during mousePressEvent()/mouseMoveEvent().
	 */
	struct CurrentChangedBlockGuard
	{
		explicit CurrentChangedBlockGuard(bool& flag) : m_flag(flag), m_previous(flag) { m_flag = true; }
		~CurrentChangedBlockGuard() { m_flag = m_previous; }
		bool& m_flag;
		bool m_previous;
	};

	TeFolderView*         mp_folderView = nullptr;    ///< Owning folder view.
	QPersistentModelIndex m_pressedIndex;   ///< Model index under the mouse at press time.
	QPoint                m_pressedPos;     ///< Cursor position at press time (for drag detection).
	QPersistentModelIndex m_anchorIndex;    ///< Explicit range-selection origin (never relies on Qt's internal anchor).
	bool                  m_currentChangedBlocked = false; ///< Suspends currentChanged() anchor sync while true.
	QRubberBand*          mp_rubberBand = nullptr;    ///< Rubber-band selection rectangle.
	TeTypes::SelectionMode m_selectionMode = TeTypes::SELECTION_NONE; ///< Current selection mode.
	int m_minFileNameWidthChars = 0; ///< Minimum non-icon filename width in "M" units.
	int m_maxFileNameWidthChars = 0; ///< Maximum non-icon filename width in "M" units.
};
