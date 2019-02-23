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

	bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);
	void requestCommandFinalize(TeCommandBase* cmdBase);

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

