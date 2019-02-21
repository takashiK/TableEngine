#pragma once
#include "commands/TeCommandBase.h"
class TeCmdMenuSetting :
	public TeCommandBase
{
public:
	TeCmdMenuSetting();
	virtual ~TeCmdMenuSetting();

protected:
	virtual bool execute(TeViewStore* p_store);
};

