/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
