/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
