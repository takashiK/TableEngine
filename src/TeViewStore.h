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
class TeFolderView;
class TeFileFolderView;
class TeArchiveFolderView;
class TeEventEmitter;
class QTabWidget;
class TeDispatcher;
class QMainWindow;
class TeDriveBar;
class QSplitter;


class TeViewStore : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	enum TabPlace {
		TAB_LEFT,
		TAB_RIGHT,
		TAB_MAX,
	};

	Q_PROPERTY(TeTypes::SelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
	Q_PROPERTY(bool driveBarVisible READ isDriveBarVisible WRITE setDriveBarVisible)
	Q_PROPERTY(bool statusBarVisible READ isStatusBarVisible WRITE setStatusBarVisible)
	Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)
	Q_PROPERTY(bool navigationVisible READ isNavigationVisible WRITE setNavigationVisible)
	Q_PROPERTY(bool detailVisible READ isDetailVisible WRITE setDetailVisible)

	Q_PROPERTY(QFlags<TeTypes::FileInfo> fileInfoFlags READ fileInfoFlags WRITE setFileInfoFlags)
	Q_PROPERTY(QFlags<TeTypes::FileType> fileTypeFlags READ fileTypeFlags WRITE setFileTypeFlags)
	Q_PROPERTY(TeTypes::OrderType fileOrderBy READ fileOrderBy WRITE setFileOrderBy)
	Q_PROPERTY(bool fileOrderReversed READ isFileOrderReversed WRITE setFileOrderReversed)
	Q_PROPERTY(TeTypes::FileViewMode viewMode READ viewMode WRITE setViewMode)


	TeViewStore(QObject *parent = Q_NULLPTR);
	virtual  ~TeViewStore();
	virtual TeTypes::WidgetType getType() const;

	void initialize();
	void show();
	void close();

	void loadMenu();
	void loadSetting();
	void loadKeySetting();

	void loadStatus();

	void setDispatcher(TeDispatcher* p_dispatcher);
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event);
	virtual void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

	QWidget*  mainWindow();


	int currentTabPlace();
	int currentTabIndex();
	int tabPlace(TeFolderView* view);
	TeFolderView* getFolderView(int place = -1);
	QList<TeFolderView*> getFolderViews(int place = -1);

	TeFolderView* currentFolderView();
	void setCurrentFolderView(TeFolderView* view);

	TeFileFolderView* createFolderView( const QString& path, int place = -1 );
	TeArchiveFolderView* createArchiveFolderView(const QString& path, int place = -1);

	void deleteFolderView( TeFolderView* view);
	void moveFolderView( TeFolderView* view, int place = 0, int position = -1);

	void changeRootPath(const QString& path, bool newView = false, int place = -1);

	void registerFloatingWidget(QWidget* widget);

	TeTypes::SelectionMode selectionMode() const { return m_selectionMode; }
	bool isDriveBarVisible() const;
	bool isStatusBarVisible() const;
	bool isToolBarVisible() const;
	bool isNavigationVisible() const;
	bool isDetailVisible() const;

	TeTypes::FileInfoFlags fileInfoFlags() const { return m_fileInfoFlags; };
	TeTypes::FileTypeFlags fileTypeFlags() const { return m_fileTypeFlags; };
	TeTypes::OrderType fileOrderBy() const { return m_fileOrderBy; };
	bool isFileOrderReversed() const { return m_fileOrderReversed; };
	TeTypes::FileViewMode viewMode() const { return m_viewMode; };

signals:
	void selectionModeChanged(TeTypes::SelectionMode mode);
	void fileListViewModeChanged(TeTypes::FileInfoFlags infoFlags, TeTypes::FileViewMode viewMode);
	void fileListShowModeChanged(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed);
	
public slots:
	void floatingWidgetClosed(QWidget* widget, QEvent* event);
	void focusFolderViewChanged(QWidget* widget, QEvent* event);
	void setSelectionMode(TeTypes::SelectionMode mode);
	void setDriveBarVisible(bool visible);
	void setStatusBarVisible(bool visible);
	void setToolBarVisible(bool visible);
	void setNavigationVisible(bool visible);
	void setDetailVisible(bool visible);

	void setFileInfoFlags(TeTypes::FileInfoFlags flags);
	void setFileTypeFlags(TeTypes::FileTypeFlags flags);
	void setFileOrderBy(TeTypes::OrderType order);
	void setFileOrderReversed(bool reversed);
	void setViewMode(TeTypes::FileViewMode mode);

protected:
	void addFolderView(TeFolderView* folderView, int place = -1);

signals:
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

private:
	TeMainWindow*        mp_mainWindow;
	QTabWidget*          mp_tab[TAB_MAX]; // 0:left 1:right
	TeDriveBar*  mp_driveBar;
	QSplitter*     mp_split;
	TeTypes::SelectionMode m_selectionMode;

	int  m_currentTabPlace;
	bool m_isNavigationVisible;

	TeTypes::FileInfoFlags m_fileInfoFlags;
	TeTypes::FileTypeFlags m_fileTypeFlags;
	TeTypes::OrderType m_fileOrderBy;
	bool m_fileOrderReversed;
	TeTypes::FileViewMode m_viewMode;

	TeDispatcher* mp_dispatcher;
	TeEventEmitter* mp_closeEventEmitter;
	TeEventEmitter* mp_focusEventEmitter;
	QList<QWidget*> m_floatingWidgets;
};
