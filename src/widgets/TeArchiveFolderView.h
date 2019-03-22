#pragma once

#include "TeFolderView.h"
#include <QPoint>
#include <QModelIndex>

class TeEventFilter;
namespace TeArchive {
	class Reader;
}

class TeArchiveFolderView : public TeFolderView
{
	Q_OBJECT
public:
	static const QString URI_WRITE;
	static const QString URI_READ;

public:
	TeArchiveFolderView(QWidget *parent = Q_NULLPTR);
	~TeArchiveFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

	void clear();
	bool setArchive(const QString& path);
	bool setArchive(TeArchive::Reader* p_archive);

protected:
	QString modelPath(QAbstractItemModel* p_model, const QModelIndex &index);
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos) const;

protected slots:
	void itemActivated(const QModelIndex &index);

private:
	enum Mode {
		MODE_WRITE,
		MODE_READ,
	};
	
	Mode m_mode;
	QString m_rootPath;

	TeFileTreeView* mp_treeView;
	TeFileListView* mp_listView;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;
};
