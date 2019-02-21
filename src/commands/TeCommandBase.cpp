#include "TeCommandBase.h"
#include "TeDispatcher.h"


TeCommandBase::TeCommandBase()
{
	mp_dispatcher = nullptr;
	mb_isFinished = false;

	m_srcType = TeTypes::WT_NONE;
	mp_srcObj = nullptr;
	mp_srcEvent = nullptr;
}


TeCommandBase::~TeCommandBase()
{
}

void TeCommandBase::setDispatcher(TeDispatcher * p_dispatcher)
{
	mp_dispatcher = p_dispatcher;
}

void TeCommandBase::setSource(TeTypes::WidgetType type, QObject * obj, QEvent * event)
{
	m_srcType = type;
	mp_srcObj = obj;
	mp_srcEvent = event;
}

void TeCommandBase::run(TeViewStore* p_store)
{
	if (execute(p_store)) {
		finished();
	}
}

void TeCommandBase::finished()
{
	if (!mb_isFinished) {
		mb_isFinished = true;
		mp_dispatcher->requestCommandFinalize(this);
	}
}

TeTypes::WidgetType TeCommandBase::srcType()
{
	return m_srcType;
}

QObject * TeCommandBase::srcObj()
{
	return mp_srcObj;
}

QEvent * TeCommandBase::srcEvent()
{
	return mp_srcEvent;
}
