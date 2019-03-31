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
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
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
	 * 実処理関数 execute()実行後も非同期処理を継続する場合はfalseを返す。
	 * 非同期処理終了時はfinished()を読んでインスタンス破棄を依頼すること。
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

