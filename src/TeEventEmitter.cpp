#include "TeEventEmitter.h"

#include <QWidget>
#include <QEvent>

TeEventEmitter::TeEventEmitter(QObject *parent)
	: QObject(parent)
{}

TeEventEmitter::~TeEventEmitter()
{}

void TeEventEmitter::addEventType(QEvent::Type eventType)
{
	m_eventTypes.append(eventType);
}

void TeEventEmitter::addOneShotEmiter(QWidget * obj)
{
	if (!m_oneShotList.contains(obj)) {
		m_oneShotList.append(obj);
		obj->installEventFilter(this);
	}
}

void TeEventEmitter::addEmitter(QWidget* obj)
{
	if (!m_objList.contains(obj)) {
		m_objList.append(obj);
		obj->installEventFilter(this);
	}
}

bool TeEventEmitter::eventFilter(QObject* obj, QEvent* event)
{
	//QEvent::Type type = event->type();
	//qDebug() << type;
	if (m_eventTypes.contains( event->type() ) ) {
		if (m_oneShotList.removeOne(obj)) {
			obj->removeEventFilter(this);
			emit emitEvent(qobject_cast<QWidget*>(obj),event);
		}
		if (m_objList.contains(obj)) {
			emit emitEvent(qobject_cast<QWidget*>(obj), event);
		}
	}
	return false;
}
