#pragma once
#include "commands/TeCommandBase.h"
class TeCmdFolderCloseUnder :
	public TeCommandBase
{
public:
	TeCmdFolderCloseUnder();
	virtual ~TeCmdFolderCloseUnder();
protected:
	virtual bool execute(TeViewStore* p_store);
};

