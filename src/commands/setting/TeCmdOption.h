#pragma once
#include "commands/TeCommandBase.h"

class TeCmdOption :
	public TeCommandBase
{
public:
	TeCmdOption();
	virtual ~TeCmdOption();

protected:
	virtual bool execute(TeViewStore* p_store);
};

