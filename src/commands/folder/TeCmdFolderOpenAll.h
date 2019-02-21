#pragma once
#include "commands/TeCommandBase.h"

class TeCmdFolderOpenAll :
	public TeCommandBase
{
public:
	TeCmdFolderOpenAll();
	virtual ~TeCmdFolderOpenAll();
protected:
	virtual bool execute(TeViewStore* p_store);
};

