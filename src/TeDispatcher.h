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

#include <stdint.h>
#include <QHash>
#include <QPair>
#include <QList>
#include "TeTypes.h"
#include "TeDispatchable.h"

/**
 * @file TeDispatcher.h
 * @brief Central event-to-command dispatcher.
 * @ingroup main
 *
 * @details Declares TeDispatcher, which maps keyboard events from widgets to
 * registered command IDs and executes them through the command queue.
 *
 * @see doc/markdown/06_dispatcher_command.md
 */

class TeCommandFactory;
class TeCommandBase;
class TeViewStore;

/**
 * @class TeDispatcher
 * @brief Routes UI events from widgets to commands.
 * @ingroup main
 *
 * @details TeDispatcher sits between widget-level event filters (TeEventFilter)
 * and the command subsystem (TeCommandBase / TeCommandFactory).  On receiving
 * a key-press from a list/tree view it looks up the matching command ID in the
 * key-binding map and enqueues execution.
 *
 * The internal command queue (@c m_cmdQueue) ensures that asynchronous commands
 * are serialised: the next command only starts after the previous one calls
 * finished().
 *
 * @see TeCommandBase, TeCommandFactory, TeViewStore
 * @see doc/markdown/06_dispatcher_command.md
 */
class TeDispatcher : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	TeDispatcher();
	virtual ~TeDispatcher();

	/** @brief Returns TeTypes::WT_NONE (the dispatcher has no widget type). */
	virtual TeTypes::WidgetType getType() const;

	/**
	 * @brief Sets the command factory used to instantiate commands.
	 * @param p_factory Pointer to the singleton TeCommandFactory.
	 */
	void setFactory(const TeCommandFactory* p_factory);

	/**
	 * @brief Sets the view store that commands operate on.
	 * @param p_store Pointer to the application-wide TeViewStore.
	 */
	void setViewStore(TeViewStore* p_store);

	/**
	 * @brief (Re-)loads key-to-command bindings from QSettings.
	 *
	 * Reads the key setting from SETTING_KEY and rebuilds @c m_keyCmdMap.
	 */
	void loadKeySetting();

	/**
	 * @brief Translates a widget event into a command execution request.
	 * @param type  The originating widget type.
	 * @param event The Qt event; only QEvent::KeyPress from list/tree views
	 *              is currently acted upon.
	 * @return Always returns true (event considered handled).
	 */
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event);

	/**
	 * @brief Requests asynchronous finalization of a running command.
	 *
	 * Emits commandFinalize(), which is connected to finishCommand().
	 * @param cmdBase The command that has completed its work.
	 */
	virtual void requestCommandFinalize(TeCommandBase* cmdBase);

signals:
	/** @brief Emitted when a command signals completion via requestCommandFinalize(). */
	void commandFinalize(TeCommandBase* cmdBase);

	/** @brief Emitted when dispatch() resolves an event to a command ID. */
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_cmdParam);

public slots:
	/**
	 * @brief Removes @p cmdBase from the queue and starts the next pending command.
	 * @param cmdBase The command that just finished.
	 */
	void finishCommand(TeCommandBase* cmdBase);

	/**
	 * @brief Instantiates and runs the command identified by @p cmdId.
	 * @param cmdId      The command to create and execute.
	 * @param type       The originating widget type.
	 * @param event      The originating event (cloned by TeCommandBase).
	 * @param p_cmdParam Optional parameters forwarded to the command.
	 */
	void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_cmdParam);

private:
	const TeCommandFactory* mp_factory;
	TeViewStore* mp_store;

	/** @brief Key-modifier + key-code to command ID mapping. */
	QHash< QPair<int,int>, TeTypes::CmdId> m_keyCmdMap;

	/** @brief Serial execution queue for asynchronous commands. */
	QList<TeCommandBase*> m_cmdQueue;
};

