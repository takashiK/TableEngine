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
