#pragma once
#include "commands/TeCommandBase.h"
class TeCmdCopyTo :
	public TeCommandBase
{
public:
	TeCmdCopyTo();
	virtual ~TeCmdCopyTo();

protected:
	virtual bool execute(TeViewStore* p_store);

	void copyItems(TeViewStore* p_store, const QStringList& list, const QString& path);
};

