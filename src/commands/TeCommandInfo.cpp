#include "TeCommandInfo.h"



TeCommandInfoBase::TeCommandInfoBase()
{
	m_cmdId = TeTypes::CMDID_NONE;
}

TeCommandInfoBase::TeCommandInfoBase(TeTypes::CmdId cmdId, const QString & name, const QString & description, const QIcon & icon)
{
	setCommandInfo(cmdId, name, description, icon);
}


TeCommandInfoBase::~TeCommandInfoBase()
{
}

void TeCommandInfoBase::setCommandInfo(TeTypes::CmdId cmdId, const QString & name, const QString & description, const QIcon & icon)
{
	m_cmdId  = cmdId;
	m_name = name;
	m_description = description;
	m_icon = icon;
}
