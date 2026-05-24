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

#include "TeFolderView.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"
#include "TeEventFilter.h"

/**
 * @file TeFolderView.cpp
 * @brief Implementation of TeFolderView.
 * @ingroup widgets
 */

#include <QLayout>
#include <QHeaderView>
#include <QStorageInfo>


TeFolderView::TeFolderView(QWidget *parent)
	: QWidget(parent)
{
	connect(this, &TeFolderView::requestCommand,
		[this](TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param) { execCommand(cmdId,type,event,p_param); });
}

TeFolderView::~TeFolderView()
{
}

void TeFolderView::setDispatcher(TeDispatchable * p_dispatcher)
{
	mp_dispatcher = p_dispatcher;
}

bool TeFolderView::dispatch(TeTypes::WidgetType type, QEvent * event)
{
	if(event->type() == QEvent::FocusIn) {
		emit focusIn();
	}
	if ((mp_dispatcher != nullptr) && isDispatchable(type, event)) {
		return mp_dispatcher->dispatch(type, event);
	}
	return false;
}

void TeFolderView::execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param)
{
	if (mp_dispatcher != nullptr) {
		mp_dispatcher->execCommand(cmdId, type, event, p_param);
	}
}
