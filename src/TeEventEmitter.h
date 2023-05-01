#pragma once

#include <QObject>
#include <QList>
#include <QEvent>

class QWidget;

class TeEventEmitter  : public QObject
{
	Q_OBJECT

public:
	TeEventEmitter(QObject *parent=nullptr);
	~TeEventEmitter();

	void addEventType(QEvent::Type eventType);

	void addOneShotEmiter(QWidget* obj);
	void addEmitter(QWidget* obj);

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event);

signals:
	void emitEvent(QWidget* obj, QEvent* event);

private:
	QList<const QWidget*> m_oneShotList;
	QList<const QWidget*> m_objList;
	QList<QEvent::Type> m_eventTypes;
};
