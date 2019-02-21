#pragma once
#include "commands/TeCommandBase.h"

class TeCmdFolderOpenOne :
	public TeCommandBase
{
public:
	TeCmdFolderOpenOne();
	virtual ~TeCmdFolderOpenOne();
protected:
	virtual bool execute(TeViewStore* p_store);
};

