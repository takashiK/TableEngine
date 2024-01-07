#pragma once
#include "commands/TeCommandBase.h"

class TeCmdToolFile :
    public TeCommandBase
{
public:
	TeCmdToolFile();
	virtual ~TeCmdToolFile();
	// Check if this command can process when item is not selected.
	static bool isActive(TeViewStore* p_store);

	// type of command
	static QFlags<TeTypes::CmdType> type();

	// Parameter list. it use for menu access.
	static QList<TeMenuParam> menuParam();

protected:
	virtual bool execute(TeViewStore* p_store);
};

