/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
 *	EventをCmdMapに従ってCommandに変換し実行する。
 */
bool TeDispatcher::dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event)
{
	//ListView/TreeView共通Dispatch
	if (type == TeTypes::WT_LISTVIEW || type == TeTypes::WT_TREEVIEW) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			//キーイベントをコマンドに変換して実行を依頼する
			TeTypes::CmdId cmdId = m_keyCmdMap.value(QPair<int,int>(keyEvent->modifiers(),keyEvent->key()));

			if (cmdId != TeTypes::CMDID_NONE) {
				emit requestCommand(cmdId,type,obj,event);
			}
		}
	}

	//Dispatcherに送られるイベントは、すべて処理完了扱いにする。
	return true;
}

/**
 * CommandIdに対応したCommandClassを生成し実行する。
 */
void TeDispatcher::execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject * obj, QEvent * event)
{
	//Command生成
	TeCommandBase* cmdBase = mp_factory->createCommand(cmdId);

	if (cmdBase != nullptr) {
		cmdBase->setDispatcher(this);

		//Source情報設定
		cmdBase->setSource(type, obj, event);

		//非同期実行用の管理キューに登録
		m_cmdQueue.push_back(cmdBase);

		//処理実行
		cmdBase->run(mp_store);
	}
}

void TeDispatcher::requestCommandFinalize(TeCommandBase * cmdBase)
{
	emit commandFinalize(cmdBase);
}

void TeDispatcher::loadKeySetting()
{
	//既存キー設定クリア
	m_keyCmdMap.clear();

	//読み込み対象キーリスト作成
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

	//設定値読み込み
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
 * 非同期コマンドの後処理(削除)を実施する。
 */
void TeDispatcher::finishCommand(TeCommandBase* cmdBase)
{
	if (cmdBase != nullptr) {
		//大丈夫だとは思うが一応多重Delete避けを入れておく
		if ( m_cmdQueue.removeAll(cmdBase) > 0 ) {
			delete cmdBase;
		}
	}
}
