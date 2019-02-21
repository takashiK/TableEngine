#pragma once

#include <QWidget>
#include <QPoint>
#include <QAbstractItemView>
#include "TeDispatchable.h"

class TeFileTreeView;
class TeFileListView;
class QFileSystemModel;
class TeEventFilter;
class TeDispatcher;

class TeFileFolderView : public QWidget, public TeDispatchable
{
	Q_OBJECT

public:
	TeFileFolderView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	void setDispatcher(TeDispatchable* p_dispatcher);
	bool dispatch(TeTypes::WidgetType type, QObject* obj, QEvent *event);

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

protected:
	bool isDispatchable(TeTypes::WidgetType type, QObject* obj, QEvent *event) const;
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos) const;

private:
	TeFileTreeView* mp_treeView;
	TeFileListView* mp_listView;

	QFileSystemModel* mp_treeModel;
	QFileSystemModel* mp_listModel;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;

	TeDispatchable* mp_dispatcher;
};
