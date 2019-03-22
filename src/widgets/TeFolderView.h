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
