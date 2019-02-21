#include "TeEventFilter.h"
#include "TeDispatcher.h"

TeEventFilter::TeEventFilter(QObject *parent)
	: QObject(parent)
{
	mp_dispatcher = nullptr;
	m_type = TeTypes::WT_NONE;
}

TeEventFilter::~TeEventFilter()
{
}

void TeEventFilter::setType(TeTypes::WidgetType type)
{
	m_type = type;
}

TeTypes::WidgetType TeEventFilter::getType()
{
	return m_type;
}

void TeEventFilter::setDispatcher(TeDispatchable * p_dispatcher)
{
	mp_dispatcher = p_dispatcher;
}

bool TeEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (mp_dispatcher != nullptr && mp_dispatcher->dispatch(m_type, obj,event)) {
		return true;
	}
	return QObject::eventFilter(obj, event);
}
