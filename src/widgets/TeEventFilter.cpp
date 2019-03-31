/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
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
