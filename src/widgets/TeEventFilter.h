#pragma once

#include <QObject>
#include <TeTypes.h>
class TeDispatchable;

class TeEventFilter : public QObject
{
	Q_OBJECT

public:
	TeEventFilter(QObject *parent = NULL);
	virtual  ~TeEventFilter();

	void setType(TeTypes::WidgetType type);
	TeTypes::WidgetType getType();

	void setDispatcher(TeDispatchable* p_dispatcher);

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	TeDispatchable* mp_dispatcher;
	TeTypes::WidgetType m_type;
};
