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

#include <QObject>
#include <TeTypes.h>

/**
 * @file TeEventFilter.h
 * @brief Event filter that routes widget events to a TeDispatchable.
 * @ingroup widgets
 */

class TeDispatchable;

/**
 * @class TeEventFilter
 * @brief QObject event filter that forwards widget events to a TeDispatchable.
 * @ingroup widgets
 *
 * @details Installed on a QAbstractItemView (typically a TeFileTreeView or
 * TeFileListView).  When an event arrives through eventFilter(), the filter
 * identifies the target widget type and calls TeDispatchable::dispatch() on
 * the configured dispatcher.
 *
 * The widget type tag (set via setType()) is embedded in every forwarded
 * event so that TeDispatcher can route the command to the correct handler.
 *
 * @see TeDispatchable, TeFolderView, TeTypes::WidgetType
 */
class TeEventFilter : public QObject
{
	Q_OBJECT

public:
	TeEventFilter(QObject *parent = NULL);
	virtual  ~TeEventFilter();

	/**
	 * @brief Sets the widget-type tag embedded in dispatched events.
	 * @param type The TeTypes::WidgetType to associate with the watched widget.
	 */
	void setType(TeTypes::WidgetType type);

	/** @brief Returns the configured widget-type tag. */
	TeTypes::WidgetType getType();

	/**
	 * @brief Sets the dispatcher target for forwarded events.
	 * @param p_dispatcher A TeDispatchable (usually the owning TeFolderView).
	 */
	void setDispatcher(TeDispatchable* p_dispatcher);

protected:
	/**
	 * @brief Intercepts events on @p obj and forwards qualifying ones.
	 * @param obj   The watched object.
	 * @param event The incoming event.
	 * @return false to let the event continue through the normal handler.
	 */
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	TeDispatchable*     mp_dispatcher; ///< Target dispatcher.
	TeTypes::WidgetType m_type;        ///< Widget-type tag used when dispatching.
};
