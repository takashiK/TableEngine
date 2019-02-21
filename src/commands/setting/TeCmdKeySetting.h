#pragma once
#include "commands/TeCommandBase.h"
class TeCmdKeySetting :
	public TeCommandBase
{
public:
	TeCmdKeySetting();
	virtual ~TeCmdKeySetting();

protected:
	virtual bool execute(TeViewStore* p_store);
};

