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

