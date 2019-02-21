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

