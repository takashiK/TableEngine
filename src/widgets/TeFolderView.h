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
	virtual bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);

	virtual void setRootPath(const QString& path) = 0;
	virtual QString rootPath() =0;
	virtual void setCurrentPath(const QString& path) =0;
	virtual QString currentPath() =0;

protected:
	bool isDispatchable(TeTypes::WidgetType type, QObject* obj, QEvent *event) const;

private:
	TeDispatchable* mp_dispatcher;
};
