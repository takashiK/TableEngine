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
