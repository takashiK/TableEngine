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

#include <QIcon>
#include <QPair>
#include <QFlags>

/**
 * @file TeCommandInfo.h
 * @brief Command metadata base class and typed template.
 * @ingroup commands
 *
 * @details Declares TeCommandInfoBase (the polymorphic metadata record stored
 * in TeCommandFactory) and the TeCommandInfo<T> template that ties a concrete
 * command class to its metadata and factory method.
 *
 * @see doc/markdown/06_dispatcher_command.md
 */

class TeViewStore;
class TeCommandBase;

/**
 * @class TeCommandInfoBase
 * @brief Polymorphic metadata record for a single command.
 * @ingroup commands
 *
 * @details Stores the display name, description, and icon for one command
 * and provides virtual helpers used by menus and toolbars to query
 * whether the command is currently active or in a selected/toggled state.
 */
class TeCommandInfoBase
{
public:

	TeCommandInfoBase();
	TeCommandInfoBase(TeTypes::CmdId type,  const QString& name, const QString& description, const QIcon& icon);
	virtual ~TeCommandInfoBase();

	/**
	 * @brief Populates the metadata fields of this descriptor.
	 * @param cmdId       The unique command identifier.
	 * @param name        Short human-readable name.
	 * @param description Longer description (used in tooltips / help text).
	 * @param icon        Icon shown in menus and toolbars.
	 */
	void setCommandInfo(TeTypes::CmdId cmdId, const QString& name, const QString& description, const QIcon& icon);

	/**
	 * @brief Returns true when the command can be executed in the current state.
	 * @param p_store The current view store.
	 */
	virtual bool isActive(TeViewStore* p_store ) const = 0;

	/**
	 * @brief Returns true when the command is currently in its "selected" state.
	 *
	 * Used for toggle commands (e.g. show hidden files) to reflect checked state
	 * in the menu.
	 * @param p_store The current view store.
	 */
	virtual bool isSelected( TeViewStore* p_store ) const = 0;

	/** @brief Returns the execution type flags for this command. */
	virtual QFlags<TeTypes::CmdType> type() const = 0;

	/**
	 * @brief Creates and returns a new heap-allocated command instance.
	 * @return Ownership is transferred to the caller.
	 */
	virtual TeCommandBase* createCommand() const = 0;

	/** @brief Returns the command identifier. */
	TeTypes::CmdId cmdId() const { return m_cmdId; }
	/** @brief Returns the short display name. */
	QString name() const { return m_name; }
	/** @brief Returns the longer description. */
	QString description() const { return m_description; }
	/** @brief Returns the command icon. */
	QIcon icon() const { return m_icon; }

protected:
	TeTypes::CmdId m_cmdId;
	QString m_name;
	QString m_description;
	QIcon   m_icon;
};

/**
 * @class TeCommandInfo
 * @brief Typed command metadata that delegates to the concrete command class T.
 * @ingroup commands
 *
 * @details The template parameter T must be a TeCommandBase subclass that
 * exposes the following static members:
 * - @c bool isActive(TeViewStore*)
 * - @c bool isSelected(TeViewStore*, const TeCmdParam*)
 * - @c QFlags<TeTypes::CmdType> type()
 *
 * @tparam T Concrete command class.
 */
template <class T> class TeCommandInfo : public TeCommandInfoBase{
public:
	TeCommandInfo() {}
	TeCommandInfo(TeTypes::CmdId type, const QString& name, const QString& description, const QIcon& icon, const TeCmdParam& cmdParam = TeCmdParam())
		: TeCommandInfoBase(type, name, description, icon), m_cmdParam(cmdParam) {}
	virtual ~TeCommandInfo() {}

	/** @brief Delegates to T::isActive(). */
	virtual bool isActive( TeViewStore* p_store ) const { return T::isActive(p_store); }
	/** @brief Delegates to T::isSelected(). */
	virtual bool isSelected( TeViewStore* p_store ) const { return T::isSelected(p_store, &m_cmdParam); }
	/** @brief Delegates to T::type(). */
	virtual QFlags<TeTypes::CmdType> type() const { return T::type(); }
	/** @brief Creates a new T instance with the stored default parameters. */
	virtual TeCommandBase* createCommand() const {  T* cmd = new T; cmd->setDefaultParam(m_cmdParam); return cmd; }

private:
	const TeCmdParam m_cmdParam;
};
