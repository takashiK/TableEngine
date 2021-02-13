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

#include <QObject>
#include <QMainWindow>
#include "TeDispatcher.h"

class TeMainWindow;
class TeFileFolderView;
class QTabWidget;
class TeDispatcher;
class QMainWindow;
class TeDriveBar;
class QSplitter;

class TeViewStore : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	enum TabIndex {
		TAB_LEFT,
		TAB_RIGHT,
		TAB_MAX,
	};

	TeViewStore(QObject *parent = Q_NULLPTR);
	virtual  ~TeViewStore();

	void initialize();
	void show();

	void loadMenu();
	void loadSetting();
	void loadKeySetting();

	void loadStatus();

	void setDispatcher(TeDispatcher* p_dispatcher);
	bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);

	virtual QWidget*  mainWindow();

	virtual int currentTabIndex();
	virtual int tabIndex(TeFileFolderView* view);
	virtual QList<TeFileFolderView*> getFolderView(int place = -1);

	virtual TeFileFolderView* currentFolderView();
	virtual void setCurrentFolderView(TeFileFolderView* view);

	virtual TeFileFolderView* createFolderView( const QString& path, int place = -1 );
	virtual void deleteFolderView( TeFileFolderView* view);
	virtual void moveFolderView( TeFileFolderView* view, int place = 0, int position = -1);


signals:
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QObject* obj, QEvent* event);

private:
	TeMainWindow*        mp_mainWindow;
	QTabWidget*          mp_tab[TAB_MAX]; // 0:left 1:right
	TeDriveBar*  mp_driveBar;
	QSplitter*     mp_split;
	TeFileFolderView* mp_currentFolderView;

	TeDispatcher* mp_dispatcher;
};
