#pragma once
#include "commands/TeCommandBase.h"

class TeFileTreeView;
class TeCmdFolderOpenUnder :
	public TeCommandBase
{
public:
	TeCmdFolderOpenUnder();
	virtual ~TeCmdFolderOpenUnder();
protected:
	virtual bool execute(TeViewStore* p_store);
	void openUnder(TeFileTreeView* p_tree, const QModelIndex& index);
};

