#pragma once
#include "commands/TeCommandBase.h"
class TeCmdSelectAll :
	public TeCommandBase
{
public:
	TeCmdSelectAll();
	virtual ~TeCmdSelectAll();

protected:
	virtual bool execute(TeViewStore* p_store);
};

