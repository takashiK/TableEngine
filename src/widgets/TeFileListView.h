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

private:
	TeFolderView*         mp_folderView;    ///< Owning folder view.
	QModelIndex           m_pressedIndex;   ///< Model index under the mouse at press time.
	QPoint                m_pressedPos;     ///< Cursor position at press time (for drag detection).
	QRubberBand*          mp_rubberBand;    ///< Rubber-band selection rectangle.
	TeTypes::SelectionMode m_selectionMode; ///< Current selection mode.
};
