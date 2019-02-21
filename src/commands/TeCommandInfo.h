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
