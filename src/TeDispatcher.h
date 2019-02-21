#pragma once

#include <stdint.h>
#include <QHash>
#include <QPair>
#include <QList>
#include "TeTypes.h"
#include "TeDispatchable.h"

class TeCommandFactory;
class TeCommandBase;
class TeViewStore;

class TeDispatcher : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	TeDispatcher();
	virtual ~TeDispatcher();

	void setFactory(const TeCommandFactory* p_factory);
	void setViewStore(TeViewStore* p_store);

	void loadKeySetting();

	bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);
	void requestCommandFinalize(TeCommandBase* cmdBase);

signals:
	void commandFinalize(TeCommandBase* cmdBase);
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject* obj, QEvent* event);

public slots:
	void finishCommand(TeCommandBase* cmdBase);
	void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject* obj, QEvent* event);

private:
	const TeCommandFactory* mp_factory;
	TeViewStore* mp_store;

	QHash< QPair<int,int>, TeTypes::CmdId> m_keyCmdMap;
	QList<TeCommandBase*> m_cmdQueue;
};

