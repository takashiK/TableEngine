#include "TeCloseEventEmitter.h"

#include <QEvent>
#include <QWidget>

TeCloseEventEmitter::TeCloseEventEmitter(QObject *parent)
	: QObject(parent)
{}

TeCloseEventEmitter::~TeCloseEventEmitter()
{}

void TeCloseEventEmitter::oneShotRegister(QWidget * obj)
{
	if (!m_objList.contains(obj)) {
		m_objList.append(obj);
		obj->installEventFilter(this);
	}
}

bool TeCloseEventEmitter::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Close) {
		if (m_objList.removeOne(obj)) {
			obj->removeEventFilter(this);
			emit closeEvent(qobject_cast<QWidget*>(obj));
		}
	}
	return false;
}
