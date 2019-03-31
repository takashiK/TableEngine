/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include "TeTypes.h"
#include <QList>
#include <QHash>

class TeCommandBase;
class TeCommandInfoBase;

class TeCommandFactory : QObject
{
	Q_OBJECT

protected:
	TeCommandFactory(QObject* parent=Q_NULLPTR);
	~TeCommandFactory();

public:
	static TeCommandFactory* factory();

	static QList<QPair<QString,TeTypes::CmdId>> groupList();
	QList<TeCommandInfoBase*> commandGroup(TeTypes::CmdId groupId);
	TeCommandInfoBase* commandInfo(TeTypes::CmdId cmdId);
	TeCommandBase* createCommand(TeTypes::CmdId cmdId) const;

private:
	QHash<TeTypes::CmdId, TeCommandInfoBase*> m_commands;
};

