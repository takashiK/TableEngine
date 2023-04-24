#pragma once
#include "commands/TeCommandBase.h"

class TeCmdViewFile :
    public TeCommandBase
{
public:
	TeCmdViewFile();
	virtual ~TeCmdViewFile();
	static bool isAvailable();

protected:
	virtual bool execute(TeViewStore* p_store);
};

