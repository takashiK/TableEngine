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
#pragma once

#include <QWidget>
#include <QPoint>
#include <QAbstractItemView>
#include "TeDispatchable.h"
#include "TeFolderView.h"

class TeFileTreeView;
class TeFileListView;
class QFileSystemModel;
class TeEventFilter;
class TeDispatcher;

class TeFileFolderView : public TeFolderView
{
	Q_OBJECT

public:
	TeFileFolderView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

protected:
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos) const;

private:
	TeFileTreeView* mp_treeView;
	TeFileListView* mp_listView;

	QFileSystemModel* mp_treeModel;
	QFileSystemModel* mp_listModel;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;

};
