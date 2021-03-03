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

#include <stdint.h>
#include <QHash>
#include <QPair>
#include <QList>
#include "TeTypes.h"
#include "TeDispatchable.h"

class TeCommandFactory;
class TeCommandBase;
class TeViewStore;

class TeDispatcher : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	TeDispatcher();
	virtual ~TeDispatcher();

	void setFactory(const TeCommandFactory* p_factory);

	void setViewStore(TeViewStore* p_store);

	void loadKeySetting();

	virtual bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);
	virtual void requestCommandFinalize(TeCommandBase* cmdBase);

signals:
	void commandFinalize(TeCommandBase* cmdBase);
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject* obj, QEvent* event);

public slots:
	void finishCommand(TeCommandBase* cmdBase);
	void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject* obj, QEvent* event);

private:
	const TeCommandFactory* mp_factory;
	TeViewStore* mp_store;

	QHash< QPair<int,int>, TeTypes::CmdId> m_keyCmdMap;
	QList<TeCommandBase*> m_cmdQueue;
};

