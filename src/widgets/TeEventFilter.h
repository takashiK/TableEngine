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
