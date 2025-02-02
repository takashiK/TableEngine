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
#include <QPoint>
#include <QAbstractItemView>
#include "TeDispatchable.h"
#include "TeFolderView.h"
#include "TeHistory.h"

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

	virtual TeTypes::WidgetType getType() const;

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

	virtual void moveNextPath();
	virtual void movePrevPath();
	virtual QStringList getPathHistory() const;

	virtual TeFinder* makeFinder();

protected:
	void updatePath(const QString& root, const QString& current=QString());
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos);
	void showUserContextMenu(const QString& menuName, const QPoint& pos);

private:
	TeFileTreeView* mp_treeView;
	TeFileListView* mp_listView;

	QFileSystemModel* mp_treeModel;
	QFileSystemModel* mp_listModel;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;

	TeHistory m_history;
};
