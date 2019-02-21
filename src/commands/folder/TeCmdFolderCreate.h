#pragma once
#include "commands/TeCommandBase.h"
class TeCmdFolderCreate :
	public TeCommandBase
{
public:
	TeCmdFolderCreate();
	virtual ~TeCmdFolderCreate();
protected:
	virtual bool execute(TeViewStore* p_store);
};

