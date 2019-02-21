#pragma once
#include "commands/TeCommandBase.h"
class TeCmdDelete :
	public TeCommandBase
{
public:
	TeCmdDelete();
	virtual ~TeCmdDelete();

protected:
	virtual bool execute(TeViewStore* p_store);
	void deleteItems(TeViewStore* p_store, const QStringList& list);
};

