#pragma once
#include "commands/TeCommandBase.h"
class TeCmdFolderCloseAll :
	public TeCommandBase
{
public:
	TeCmdFolderCloseAll();
	virtual ~TeCmdFolderCloseAll();
protected:
	virtual bool execute(TeViewStore* p_store);
};

