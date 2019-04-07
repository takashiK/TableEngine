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
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
