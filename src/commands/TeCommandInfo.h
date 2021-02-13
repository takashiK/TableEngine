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
class TeCommandBase;

class TeCommandInfoBase
{
public:

	TeCommandInfoBase();
	TeCommandInfoBase(TeTypes::CmdId type,  const QString& name, const QString& description, const QIcon& icon);
	virtual ~TeCommandInfoBase();

	void setCommandInfo(TeTypes::CmdId cmdId, const QString& name, const QString& description, const QIcon& icon);

	virtual TeCommandBase* createCommand() const = 0;
	TeTypes::CmdId cmdId() const { return m_cmdId; }
	QString name() const { return m_name; }
	QString description() const { return m_description; }
	QIcon icon() const { return m_icon; }

protected:
	TeTypes::CmdId m_cmdId;
	QString m_name;
	QString m_description;
	QIcon   m_icon;
};

template <class T> class TeCommandInfo : public TeCommandInfoBase{
public:
	TeCommandInfo() {}
	TeCommandInfo(TeTypes::CmdId type, const QString& name, const QString& description, const QIcon& icon)
		: TeCommandInfoBase(type, name, description, icon) {}
	virtual ~TeCommandInfo() {}

	virtual TeCommandBase* createCommand() const { return new T; }
};
