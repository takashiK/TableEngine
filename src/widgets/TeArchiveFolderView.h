#pragma once

#include "TeFolderView.h"

class TeArchiveFolderView : public TeFolderView
{
	Q_OBJECT

public:
	TeArchiveFolderView(QWidget *parent = Q_NULLPTR);
	~TeArchiveFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

private:
};
