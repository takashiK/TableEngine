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
#include "TeTypes.h"

/**
 * @file TeCommandBase.h
 * @brief Abstract base class for all TableEngine commands.
 * @ingroup commands
 *
 * @details Defines the lifecycle contract every concrete command must fulfil.
 *
 * @see doc/markdown/06_dispatcher_command.md
 */

class TeDispatcher;
class TeViewStore;

/**
 * @class TeCommandBase
 * @brief Abstract base class providing the common command lifecycle.
 * @ingroup commands
 *
 * @details Every user-visible action in TableEngine is represented as a
 * subclass of TeCommandBase.  The command pattern allows both synchronous
 * (execute() returns true) and asynchronous (execute() returns false) commands.
 *
 * **Synchronous command flow**
 * 1. TeDispatcher::execCommand() creates the command via TeCommandFactory.
 * 2. setSource() records the originating widget type and event.
 * 3. run() calls execute().
 * 4. execute() performs its work and returns true — the command is
 *    automatically finalized.
 *
 * **Asynchronous command flow**
 * 1–3. Same as above.
 * 4. execute() starts background work and returns false — the command
 *    remains in TeDispatcher's queue.
 * 5. When background work completes, the command calls finished(), which
 *    triggers TeDispatcher to remove it from the queue and start the next
 *    pending command.
 *
 * @see TeDispatcher, TeCommandFactory, doc/markdown/06_dispatcher_command.md
 */
class TeCommandBase
{
public:
	TeCommandBase();
	virtual ~TeCommandBase();

	/**
	 * @brief Sets the dispatcher used to signal completion.
	 * @param p_dispatcher The application-wide TeDispatcher.
	 */
	void setDispatcher(TeDispatcher* p_dispatcher);

	/**
	 * @brief Sets a default parameter bundle for this command instance.
	 * @param param The parameter map to store.
	 */
	void setDefaultParam(const TeCmdParam& param);

	/**
	 * @brief Captures the source context before execute() is called.
	 * @param type       The widget type that triggered the command.
	 * @param event      The originating Qt event (cloned internally).
	 * @param p_cmdParam Optional extra parameters (merged into m_cmdParam).
	 */
	void setSource(TeTypes::WidgetType type, QEvent *event, const TeCmdParam* p_cmdParam);

	/**
	 * @brief Entry point called by TeDispatcher to start the command.
	 * @param p_store The application-wide TeViewStore.
	 */
	void run(TeViewStore* p_store);

protected:
	/**
	 * @brief Override to implement the command's main logic.
	 *
	 * Return false for asynchronous commands; call finished() when done.
	 * Return true for synchronous commands.
	 *
	 * @param p_store The application-wide TeViewStore.
	 * @return true when work is complete; false when still running.
	 */
	virtual bool execute(TeViewStore* p_store ) = 0;

	/**
	 * @brief Signals TeDispatcher that this command has finished.
	 *
	 * Must be called by asynchronous commands when background work is done.
	 * After this call the command instance will be deleted by the dispatcher.
	 */
	void finished();

	/** @brief Returns the widget type from which this command was triggered. */
	TeTypes::WidgetType srcType() const;

	/** @brief Returns the parameter bundle for this command. */
	const TeCmdParam*   cmdParam() const;

	/** @brief Returns the originating Qt event (clone owned by this object). */
	const QEvent*       srcEvent() const;

private:
	bool mb_isFinished;
	TeDispatcher* mp_dispatcher;
	TeTypes::WidgetType m_srcType;
	TeCmdParam          m_cmdParam;
	QEvent*             mp_srcEvent;
};

