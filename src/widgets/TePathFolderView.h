#pragma once

#include "widgets/TeFolderView.h"
#include "TeHistory.h"
#include "TeFileInfo.h"

class TeEventFilter;
class QAbstractItemView;
class QStandardItem;
class TeFileInfoAcsr;

class TePathFolderView : public TeFolderView
{
	Q_OBJECT

	enum Role {
		ROLE_EXCHANGE = TeFileInfo::ROLE_USER_START,
	};

public:
	TePathFolderView(QWidget *parent = nullptr);
	~TePathFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

	virtual void moveNextPath();
	virtual void movePrevPath();
	virtual QStringList getPathHistory() const;

	void clear();
	QStandardItem* createItem(const QIcon& icon, const QString& title, TeFileInfoAcsr* p_acsr=nullptr);

protected:
	void changeListView(const QModelIndex& index);
	void showContextMenu(const QAbstractItemView* p_view, const QPoint& pos);

protected slots:
	void itemActivated(const QModelIndex& index);

private:
	TeFileListView* mp_listView;
	TeFileTreeView* mp_treeView;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;

	TeHistory m_history;
};
