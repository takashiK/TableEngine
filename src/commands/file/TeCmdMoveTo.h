#pragma once
#include "commands/TeCommandBase.h"
class TeCmdMoveTo :
	public TeCommandBase
{
public:
	TeCmdMoveTo();
	virtual ~TeCmdMoveTo();

protected:
	virtual bool execute(TeViewStore* p_store);
	void moveItems(TeViewStore* p_store, const QStringList& list, const QString& path);
};

