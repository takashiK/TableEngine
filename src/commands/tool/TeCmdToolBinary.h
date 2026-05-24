#pragma once
#include "commands/TeCommandBase.h"

/**
 * @file TeCmdToolBinary.h
 * @brief Declaration of TeCmdToolBinary.
 * @ingroup commands
 */


class TeCmdToolBinary :
    public TeCommandBase
{
public:
	TeCmdToolBinary();
	virtual ~TeCmdToolBinary();
	// Check if this command is selected when item is selected.
	static bool isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam);


	// type of command
	static QFlags<TeTypes::CmdType> type();

protected:
	virtual bool execute(TeViewStore* p_store);
};

