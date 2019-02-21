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

	void createWindow();
	void show();

	void loadMenu();

	void loadStatus();

	void loadSetting();

	void loadKeySetting();

	void setDispatcher(TeDispatcher* p_dispatcher);
	bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);

	virtual QMainWindow*  mainWindow();

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
