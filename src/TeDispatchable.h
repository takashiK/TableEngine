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

#include <QEvent>
#include <QObject>
#include "TeTypes.h"

/**
 * @file TeDispatchable.h
 * @brief Interface for objects that can receive dispatched commands.
 * @ingroup main
 *
 * @details Declares the pure-virtual interface that every widget participating
 * in the event/command routing chain must implement.  Both TeDispatcher and
 * TeFolderView implement this interface.
 *
 * @see doc/markdown/06_dispatcher_command.md
 */

/**
 * @class TeDispatchable
 * @brief Pure-virtual interface for command-dispatchable objects.
 * @ingroup main
 *
 * @details Any widget or store that wants to participate in the
 * TeDispatcher routing chain must inherit from this interface and
 * implement all three pure-virtual methods.
 *
 * @see TeDispatcher, TeFolderView
 */
class TeDispatchable
{
public:

	TeDispatchable()
	{
	}

	virtual ~TeDispatchable()
	{
	}

	/**
	 * @brief Returns the widget type identifier of this object.
	 * @return A TeTypes::WidgetType value identifying the concrete type.
	 */
	virtual TeTypes::WidgetType getType() const = 0;

	/**
	 * @brief Dispatches a UI event to the appropriate command.
	 * @param type  The widget type that originally produced the event.
	 * @param event The Qt event to dispatch.
	 * @return true when the event was fully handled; false otherwise.
	 */
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event) = 0;

	/**
	 * @brief Executes a command identified by @p cmdId.
	 * @param cmdId   The command identifier to execute.
	 * @param type    The widget type that triggered the command.
	 * @param event   The originating Qt event (may be nullptr).
	 * @param p_param Optional extra parameters for the command.
	 */
	virtual void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param) = 0;
};

