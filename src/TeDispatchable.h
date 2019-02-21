#pragma once

#include <QEvent>
#include <QObject>
#include "TeTypes.h"

class TeDispatchable
{
public:

	TeDispatchable()
	{
	}

	virtual ~TeDispatchable()
	{
	}

	virtual bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event) = 0;
};

