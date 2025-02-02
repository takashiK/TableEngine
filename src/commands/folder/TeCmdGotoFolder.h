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

#include "commands/TeCommandBase.h"

#include <QList>
#include <QFlags>

class TeViewStore;

class TeCmdGotoFolder :
    public TeCommandBase
{
public:
	TeCmdGotoFolder();
	virtual ~TeCmdGotoFolder();

	// Check if this command can process when item is not selected.
	static bool isActive(TeViewStore* p_store);

	// type of command
	static QFlags<TeTypes::CmdType> type();

	// Parameter list. it use for menu access.
	static QList<TeMenuParam> menuParam();

protected:
	// Execute command.
	// return
	//   true  : command is finished. after execute, this command is deleted.
	//   false : command is still processing. after finish. you must call finish() to delete this command.
	virtual bool execute(TeViewStore* p_store);
};

