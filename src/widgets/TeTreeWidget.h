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

#include <QTreeWidget>

/**
 * @file TeTreeWidget.h
 * @brief QTreeWidget subclass with drag-and-drop move between two linked widgets.
 * @ingroup widgets
 */

/**
 * @class TeTreeWidget
 * @brief QTreeWidget subclass that supports drag-and-drop item moves to a
 *        "buddy" tree widget.
 * @ingroup widgets
 *
 * @details Used for two-pane list editors (e.g. the key-binding dialog) where
 * items can be dragged between two TeTreeWidget instances.  A buddy reference
 * is set via setBuddy(); drop events route to the buddy when appropriate.
 */
class TeTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	TeTreeWidget(QWidget *parent = nullptr);
	virtual ~TeTreeWidget();

	/**
	 * @brief Sets the partner tree widget for drag-and-drop operations.
	 * @param p_buddy Pointer to the paired TeTreeWidget.
	 */
	void setBuddy(QTreeWidget* p_buddy);

protected:
	/**
	 * @brief Accepts drag-enter events carrying QTreeWidgetItem MIME data.
	 * @param event The drag-enter event.
	 */
	virtual void dragEnterEvent(QDragEnterEvent* event);

	/**
	 * @brief Handles drops, moving items between this widget and the buddy.
	 * @param event The drop event.
	 */
	virtual void dropEvent(QDropEvent* event);

private:
	QTreeWidget* mp_buddy; ///< Linked partner tree widget for D&D moves.
};
