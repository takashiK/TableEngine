#pragma once
#include "commands/TeCommandBase.h"
class TeCmdSelectToggle :
	public TeCommandBase
{
public:
	TeCmdSelectToggle();
	virtual ~TeCmdSelectToggle();

protected:
	virtual bool execute(TeViewStore* p_store);
};

