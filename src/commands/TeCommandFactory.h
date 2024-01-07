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

class TeCommandBase;
class TeCommandInfoBase;

struct TeMenuSpec {
	TeTypes::CmdId cmdId;
	QString        name;
	int            rank;
};

class TeCommandFactory : QObject
{
	Q_OBJECT

protected:
	TeCommandFactory();
	~TeCommandFactory();

public:
	static TeCommandFactory* factory();

	static QList<QPair<QString,TeTypes::CmdId>> groupList();
	static QList<QPair<QString, TeTypes::CmdId>> custom_groupList();
	static QList<QPair<QString, TeTypes::CmdId>> static_groupList();

	QList<const TeCommandInfoBase*> commandGroup(TeTypes::CmdId groupId) const;
	QList<TeMenuSpec> menuGroup(TeTypes::CmdId groupId) const;

	const TeCommandInfoBase* commandInfo(TeTypes::CmdId cmdId) const;
	TeCommandBase* createCommand(TeTypes::CmdId cmdId) const;

private:
	QMap<TeTypes::CmdId, TeCommandInfoBase*> m_commands;
	QMap<TeTypes::CmdId, QList<TeMenuSpec>> m_menuSpecs;
	QMap<TeTypes::CmdId, QList<TeTypes::CmdId>> m_group;
};

