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

#include <QWidget>
#include <QKeyEvent>
#include <QSettings>
#include <QKeySequence>
#include "TeSettings.h"

#include "TeDispatcher.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandBase.h"

TeDispatcher::TeDispatcher()
{
	mp_factory = nullptr;
	mp_store = nullptr;
	connect(this, SIGNAL(commandFinalize(TeCommandBase*)), this, SLOT(finishCommand(TeCommandBase*)));
	connect(this, SIGNAL(requestCommand(TeTypes::CmdId, TeTypes::WidgetType, QObject*, QEvent*)), this, SLOT(execCommand(TeTypes::CmdId, TeTypes::WidgetType, QObject*, QEvent*)));

	loadKeySetting();
}


TeDispatcher::~TeDispatcher()
{
}

void TeDispatcher::setFactory(const TeCommandFactory* p_factory)
{
	mp_factory = p_factory;
}

void TeDispatcher::setViewStore(TeViewStore * p_store)
{
	mp_store = p_store;
}

/**
 *	dispatch event to command.
 */
bool TeDispatcher::dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event)
{
	//For ListView & TreeView
	if (type == TeTypes::WT_LISTVIEW || type == TeTypes::WT_TREEVIEW) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			//change key event to command Id.
			TeTypes::CmdId cmdId = m_keyCmdMap.value(QPair<int,int>(keyEvent->modifiers(),keyEvent->key()));

			if (cmdId != TeTypes::CMDID_NONE) {
				emit requestCommand(cmdId,type,obj,event);
			}
		}
	}

	//all events mark "finished"
	return true;
}

/**
 * execute command by commad Id
 */
void TeDispatcher::execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject * obj, QEvent * event)
{
	TeCommandBase* cmdBase = mp_factory->createCommand(cmdId);

	if (cmdBase != nullptr) {
		cmdBase->setDispatcher(this);

		cmdBase->setSource(type, obj, event);

		//command queuing that use to asynchronus execution.
		//this queue entry clear when command call requestCommandFinalize that is called by TeCommandBase::finished().
		m_cmdQueue.push_back(cmdBase);

		cmdBase->run(mp_store);
	}
}

void TeDispatcher::requestCommandFinalize(TeCommandBase * cmdBase)
{
	emit commandFinalize(cmdBase);
}

void TeDispatcher::loadKeySetting()
{
	m_keyCmdMap.clear();

	QList<int> list;
	for (uint32_t key = Qt::Key_A; key <= Qt::Key_Z; key++) {
		list.append(key);
		list.append(Qt::CTRL + key);
	}
	for (uint32_t key = Qt::Key_0; key <= Qt::Key_9; key++) {
		list.append(key);
		list.append(Qt::CTRL + key);
	}
	for (uint32_t key = Qt::Key_F1; key <= Qt::Key_F12; key++) {
		list.append(key);
		list.append(Qt::CTRL + key);
		list.append(Qt::SHIFT + key);
	}
	list.append(Qt::Key_Escape);
	list.append(Qt::Key_Backspace);
	list.append(Qt::Key_Delete);

	QSettings settings;
	settings.beginGroup(SETTING_KEY);
	const int modifier = Qt::CTRL | Qt::SHIFT;

	for (const auto& key : list) {
		int cmdId = settings.value(QKeySequence(key).toString().replace("+", "_")).toInt();
		if (cmdId != TeTypes::CMDID_NONE) {
			m_keyCmdMap.insert(QPair<int, int>((key & modifier), (key & 0xFF)), static_cast<TeTypes::CmdId>(cmdId));
		}
	}
}

/**
 * clear asynchronous execution qeuue.
 */
void TeDispatcher::finishCommand(TeCommandBase* cmdBase)
{
	if (cmdBase != nullptr) {
		//
		if ( m_cmdQueue.removeAll(cmdBase) > 0 ) {
			delete cmdBase;
		}
	}
}
