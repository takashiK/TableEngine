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
#pragma once
#include <QObject>
#include "TeTypes.h"

class TeDispatcher;
class TeViewStore;

class TeCommandBase
{
public:
	TeCommandBase();
	virtual ~TeCommandBase();
	void setDispatcher(TeDispatcher* p_dispatcher);
	void setSource(TeTypes::WidgetType type, QObject* obj, QEvent *event);
	void run(TeViewStore* p_store);

protected:
	/**
	 * Main Processing Function: execute() need return false if it is continuous proceeding.
	 * this type command is called "Asynchronized Command".
	 * When finish asynchronized command then you should call finished().
	 * if "finished()" is called then Dispatcher delete class instance of asynchronized command.
	 */
	virtual bool execute(TeViewStore* p_store ) = 0;
	void finished();
	TeTypes::WidgetType srcType();
	QObject*            srcObj();
	QEvent*             srcEvent();

private:
	bool mb_isFinished;
	TeDispatcher* mp_dispatcher;
	TeTypes::WidgetType m_srcType;
	QObject*            mp_srcObj;
	QEvent*             mp_srcEvent;
};

