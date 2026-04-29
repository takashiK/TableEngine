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

#include "TeTypes.h"
#include <QList>
#include <QMap>
#include <QString>

/**
 * @file TeCommandFactory.h
 * @brief Singleton factory and registry for all application commands.
 * @ingroup commands
 *
 * @details Provides the central registry that maps TeTypes::CmdId values to
 * TeCommandInfoBase instances and creates command objects on demand.
 *
 * @see doc/markdown/06_dispatcher_command.md
 */

class TeCommandBase;
class TeCommandInfoBase;

/**
 * @brief Metadata structure used to build context-menu and toolbar items.
 */
struct TeMenuSpec {
	TeTypes::CmdId cmdId; ///< Command identifier.
	QString        name;  ///< Display name shown in the menu.
	int            rank;  ///< Sort rank within the group.
};

/**
 * @class TeCommandFactory
 * @brief Singleton registry and factory for TableEngine commands.
 * @ingroup commands
 *
 * @details TeCommandFactory is constructed once during application startup.
 * Every command class self-registers by adding a TeCommandInfo<T> instance
 * to the factory.  At runtime TeDispatcher calls createCommand() to
 * instantiate a concrete command object by ID.
 *
 * @see TeCommandBase, TeCommandInfo, doc/markdown/06_dispatcher_command.md
 */
class TeCommandFactory : QObject
{
	Q_OBJECT

protected:
	TeCommandFactory();
	~TeCommandFactory();

public:
	/** @brief Returns the singleton factory instance. */
	static TeCommandFactory* factory();

	/** @brief Returns both built-in and user-registered command groups. */
	static QList<QPair<QString,TeTypes::CmdId>> groupList();

	/** @brief Returns only user-registered (custom) command groups. */
	static QList<QPair<QString, TeTypes::CmdId>> custom_groupList();

	/** @brief Returns only built-in (static) command groups. */
	static QList<QPair<QString, TeTypes::CmdId>> static_groupList();

	/**
	 * @brief Returns metadata for all commands in @p groupId.
	 * @param groupId The group command ID (e.g. CMDID_SYSTEM_FILE).
	 */
	QList<const TeCommandInfoBase*> commandGroup(TeTypes::CmdId groupId) const;

	/**
	 * @brief Returns menu-spec entries for all commands in @p groupId.
	 * @param groupId The group command ID.
	 */
	QList<TeMenuSpec> menuGroup(TeTypes::CmdId groupId) const;

	/**
	 * @brief Returns the metadata descriptor for @p cmdId.
	 * @param cmdId The command to look up.
	 * @return Pointer to the descriptor, or nullptr if not registered.
	 */
	const TeCommandInfoBase* commandInfo(TeTypes::CmdId cmdId) const;

	/**
	 * @brief Instantiates and returns a new command object for @p cmdId.
	 * @param cmdId The command to instantiate.
	 * @return A heap-allocated TeCommandBase subclass, or nullptr.
	 */
	TeCommandBase* createCommand(TeTypes::CmdId cmdId) const;

private:
	QMap<TeTypes::CmdId, TeCommandInfoBase*>          m_commands;
	QMap<TeTypes::CmdId, QList<TeMenuSpec>>            m_menuSpecs;
	QMap<TeTypes::CmdId, QList<TeTypes::CmdId>>        m_group;
};

