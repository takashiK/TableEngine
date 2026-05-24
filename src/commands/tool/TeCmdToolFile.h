#pragma once
#include "commands/TeCommandBase.h"

/**
 * @file TeCmdToolFile.h
 * @brief Declaration of TeCmdToolFile.
 * @ingroup commands
 */


class TeCmdToolFile :
    public TeCommandBase
{
public:
	TeCmdToolFile();
	virtual ~TeCmdToolFile();
	// Check if this command is selected when item is selected.
	static bool isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam);


	// type of command
	static QFlags<TeTypes::CmdType> type();

protected:
	virtual bool execute(TeViewStore* p_store);
};

