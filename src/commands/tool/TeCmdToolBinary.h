#pragma once
#include "commands/TeCommandBase.h"

class TeCmdToolBinary :
    public TeCommandBase
{
public:
	TeCmdToolBinary();
	virtual ~TeCmdToolBinary();
	// Check if this command can process when item is not selected.
	static bool isActive(TeViewStore* p_store);
	// Check if this command is selected when item is selected.
	static bool isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam);


	// type of command
	static QFlags<TeTypes::CmdType> type();

protected:
	virtual bool execute(TeViewStore* p_store);
};

