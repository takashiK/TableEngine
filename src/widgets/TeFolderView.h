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

#include <QWidget>
#include "TeDispatchable.h"

class TeFileTreeView;
class TeFileListView;

class TeFolderView : public QWidget, public TeDispatchable
{
	Q_OBJECT

public:
	TeFolderView(QWidget *parent);
	~TeFolderView();
	virtual TeFileTreeView* tree() = 0;
	virtual TeFileListView* list() = 0;

	virtual void setDispatcher(TeDispatchable* p_dispatcher);
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event);
	virtual void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

	virtual void setRootPath(const QString& path) = 0;
	virtual QString rootPath() =0;
	virtual void setCurrentPath(const QString& path) =0;
	virtual QString currentPath() =0;

signals:
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

protected:
	bool isDispatchable(TeTypes::WidgetType type, QEvent *event) const;

private:
	TeDispatchable* mp_dispatcher;
};
